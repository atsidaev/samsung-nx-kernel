/**
 * @file d4_ep.c
 * @brief DRIMe4 EP Platform Driver
 * @author Wooram Son <wooram.son@samsung.com>
 * 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/io.h>

#include <mach/ep/d4_ep.h>

#ifdef CONFIG_PMU_SELECT
#include <media/drime4/opener/d4_opener_ioctl.h>
#endif

#ifdef CONFIG_PMU_SELECT
#include <linux/d4-pmu.h>
#endif

#ifdef CONFIG_RMU_DEV
#include <linux/d4_rmu.h>
#endif

#include "d4_ep_if.h"
#include "d4_ep_dd.h"
#include "d4_ep_regs.h"

extern const struct file_operations drime4_ep_ops;

/**
 * @brief   EP의 Miscellaneous Device 설정을 위한 miscdevice 구조체 설정
 * @author  Wooram Son
 */
static struct miscdevice drime4_ep_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = EP_MODULE_NAME,
	.fops = &drime4_ep_ops,
};

/**
 * @brief EP의 Data Structure
 */
struct drime4_ep *g_ep;

static char *ep_core_intr_name[] = {
	"EP_LDC_TILE_FINISH",
	"EP_LDC_OPERATION_FINISH",
	"EP_LDC_ERROR",
	"EP_HDR_TILE_FINISH",
	"EP_HDR_OPERATION_FINISH",
	"EP_HDR_ERROR",
	"EP_NRm_TILE_FINISH",
	"EP_NRm_OPERATION_FINISH",
	"EP_NRm_ERROR",
	"EP_MnF_TILE_FINISH",
	"EP_MnF_OPERATION_FINISH",
	"EP_MnF_ERROR",
	"EP_FD_TILE_FINISH",
	"EP_FD_OPERATION_FINISH",
	"EP_FD_ERROR",
	"EP_BITBLT_OPERATION_FINISH",
	"EP_BITBLT_ERROR",
	"EP_LVR_VIDEO_Y_TILE_FINISH",
	"EP_LVR_VIDEO_Y_OPERATION_FINISH",
	"EP_LVR_VIDEO_ERROR",
	"EP_LVR_GRP_OPERATION_FINISH",
	"EP_LVR_GRP_ERROR",
	"EP_OTF_TILE_FINISH",
	"EP_OTF_OPERATION_FINISH",
	"EP_TOP_ERROR",
	"EP_LVR_VIDEO_C_TILE_FINISH",
	"EP_LVR_VIDEO_C_OPERATION_FINISH"
};

static char *ep_dma_intr_name[] = {
	"EP_WDMA0_INPUT_END_NRm",
	"EP_WDMA0_BUS_END_NRm",
	"EP_WDMA0_ERROR_NRm",
	"Reserved",
	"EP_WDMA1_INPUT_END_MnF",
	"EP_WDMA1_BUS_END_MnF",
	"EP_WDMA1_ERROR_MnF",
	"Reserved",
	"EP_WDMA2_INPUT_END_BBLT",
	"EP_WDMA2_BUS_END_BBLT",
	"EP_WDMA2_ERROR_BBLT",
	"Reserved",
	"EP_WDMA0_VSYNC_END_NRm",
	"EP_WDMA1_VSYNC_END_MnF1",
	"EP_WDMA2_VSYNC_END_MnF2",
	"EP_WDMA3_VSYNC_END_BBLT",
	"EP_SM_SELF_DONE0",
	"EP_SM_SELF_ERROR",
	"EP_DMA_INTR_MAX"
};

/**
 * @brief EP의 IRQ Handler
 * @fn      static irqreturn_t drime4_ep_irq(int irq, void *dev_id)
 * @param   irq		[in] irq number
 * @param   *dev_id	[in] dev_id
 * @return  irqreturn_t	인터럽트 처리 여부
 *
 * @author  Wooram Son
 */
static irqreturn_t drime4_ep_irq(int irq, void *dev_id)
{
	int i;
	unsigned int status, clear;
	struct drime4_ep *ep;
	unsigned long flags;

	local_save_flags(flags);
	local_irq_disable();

	ep = g_ep;
	EP_INTR_DEBUG_MSG("[EP IRQ Handler %d] %s\n", irq, ep->name);

	/**
	 * @brief EP CORE IRQ 처리
	 */
	if (irq == IRQ_EP_CORE) {
		status = READ_EPTOP_REG(ep->reg_base_top, D4_EP_TOP_INT_STATUS_IP);

		for (i = 0; i < EP_K_MAX_INTR; i++) {
			if (status & (0x1 << i)) {
				/**
				 * @brief 해당 인터럽트 Clear
				 */
				clear = status | (0x1 << i);
				__raw_writel(clear, ep->reg_base_top + D4_EP_TOP_INT_CLEAR_IP);

				EP_INTR_DEBUG_MSG("[EP IRQ Handler %d] Clear bit %d (%s)\n", irq, i, ep_core_intr_name[i]);

				if (i == EP_K_LDC_ERROR || i == EP_K_HDR_ERROR
						|| i == EP_K_NRm_ERROR || i == EP_K_MnF_ERROR
						|| i == EP_K_FD_ERROR || i == EP_K_BITBLT_ERROR
						|| i == EP_K_LVR_VIDEO_ERROR || i == EP_K_LVR_GRP_ERROR
						|| i == EP_K_TOP_ERROR)
					EP_INTR_ERROR_MSG("[EP IRQ Handler %d] Core Error %d (%s)\n", irq, i,
							ep_core_intr_name[i]);

				/**
				 * @brief WaitQueue를 이용하여 특정 인터럽트를 대기하는 프로세스가 있다면 Wake Up 해주는 작업
				 */
				ep_top_wakeup_core_intr(i);
			}
		}
	}

	/**
	 * @brief EP DMA IRQ 처리
	 */
	if (irq == IRQ_EP_DMA) {
		status = READ_EPTOP_REG(ep->reg_base_top, D4_EP_TOP_INT_STATUS_DMA);

		for (i = 0; i < EP_K_DMA_INTR_MAX; i++) {
			if (status & (0x1 << i)) {
				/**
				 * @brief 해당 인터럽트 Clear
				 */
				clear = status | (0x1 << i);
				__raw_writel(clear, ep->reg_base_top + D4_EP_TOP_INT_CLEAR_DMA);

				EP_INTR_DEBUG_MSG("[EP IRQ Handler %d] Clear bit %d (%s)\n", irq, i, ep_dma_intr_name[i]);

				if (i == EP_K_WDMA0_ERROR_NRm || i == EP_K_WDMA1_ERROR_MnF
						|| i == EP_K_WDMA2_ERROR_BBLT || i == EP_K_SM_SELF_ERROR)
					EP_INTR_ERROR_MSG("[EP IRQ Handler %d] DMA Error %d (%s)\n", irq, i,
							ep_dma_intr_name[i]);

				/**
				 * @brief WaitQueue를 이용하여 특정 인터럽트를 대기하는 프로세스가 있다면 Wake Up 해주는 작업
				 */
				ep_top_wakeup_dma_intr(i);
			}
		}
	}
	local_irq_restore(flags);

	return IRQ_HANDLED;
}

/**
 * @brief EP의 Sub-IP 별로 해당 레지스터의 레지스터 mem_region 설정
 * @fn      static int drime4_ep_register_iomap(struct platform_device *pdev,
		struct drime4_ep *ep, int res_num)
 * @param   *pdev	[in] platform device 구조체
 * @param   *ep		[in] EP 컨텍스트 구조체
 * @param   res_num	[in] 리소스 번호 (Sub-IP 구별을 위해 사용)
 * @return  int	mem_region 할당 실패시 음수값 반환
 *
 * @author  Wooram Son
 */
static int drime4_ep_register_iomap(struct platform_device *pdev,
		struct drime4_ep *ep, int res_num)
{
	struct resource *res = NULL;
	void __iomem *regs;
	int ret = 0;

	/**
	 * @brief get resource for io memory
	 */
	res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);
	if (!res) {
		dev_err(&pdev->dev, "failed to get io memory region\n");
		ret = -ENODEV;
		goto out;
	}

	/**
	 * @brief request mem region
	 */
	res = request_mem_region(res->start, resource_size(res), pdev->name);
	if (!res) {
		dev_err(&pdev->dev, "failed to request io memory region\n");
		ret = -ENOMEM;
		goto out;
	}

	/**
	 * @brief ioremap for register block
	 */
	regs = ioremap(res->start, resource_size(res));
	if (!regs) {
		dev_err(&pdev->dev, "failed to remap io region\n");
		ret = -ENOMEM;
		goto err_no_ioremap;
	}

	/**
	 * @brief 해당 Sub-IP에 Base Register Address를 전달
	 */
	if (res_num == 0) {
		ep->reg_base_top = regs;
		ep_set_top_reg_info((unsigned int)res->start, (unsigned int)resource_size(res));
		ep_dd_set_top_reg(regs);
	} else if (res_num == 1) {
		ep->reg_base_ldc = regs;
		ep_set_ldc_reg_info((unsigned int)res->start, (unsigned int)resource_size(res));
	} else if (res_num == 2) {
		ep->reg_base_hdr = regs;
		ep_set_hdr_reg_info((unsigned int)res->start, (unsigned int)resource_size(res));
	} else if (res_num == 3) {
		ep->reg_base_nrm = regs;
		ep_set_nrm_reg_info((unsigned int)res->start, (unsigned int)resource_size(res));
	} else if (res_num == 4) {
		ep->reg_base_mnf = regs;
		ep_set_mnf_reg_info((unsigned int)res->start, (unsigned int)resource_size(res));
	} else if (res_num == 5) {
		ep->reg_base_fd = regs;
		ep_set_fd_reg_info((unsigned int)res->start, (unsigned int)resource_size(res));
	} else if (res_num == 6) {
		ep->reg_base_bblt = regs;
		ep_set_bblt_reg_info((unsigned int)res->start, (unsigned int)resource_size(res));
	} else if (res_num == 7) {
		ep->reg_base_lvr = regs;
		ep_set_lvr_reg_info((unsigned int)res->start, (unsigned int)resource_size(res));
	} else if (res_num == 8) {
		ep->reg_base_dma = regs;
		ep_set_dma_reg_info((unsigned int)res->start, (unsigned int)resource_size(res));
		ep_dd_set_dma_reg(regs);
	}

	return 0;

err_no_ioremap:
	release_mem_region(res->start, resource_size(res));
out:
	return ret;
}

/**
 * @brief EP의 Sub-IP 별로 해당 레지스터의 레지스터 mem_region 설정
 * @fn      static int drime4_ep_register_release_iomap(struct platform_device *pdev, int res_num)
 * @param   *pdev	[in] platform device 구조체
 * @param   *ep		[in] EP 컨텍스트 구조체
 * @param   res_num	[in] 리소스 번호 (Sub-IP 구별을 위해 사용)
 * @return  mem_region 해제 실패시 음수 값 반환
 *
 * @author  Wooram Son
 */
static int drime4_ep_register_release_iomap(struct platform_device *pdev, int res_num)
{
   struct resource *res = NULL;
   int ret = 0;

   /* get resource for io memory  */
   res = platform_get_resource(pdev, IORESOURCE_MEM, res_num);
   if (!res) {
      dev_err(&pdev->dev, "failed to get io memory region\n");
      ret = -ENODEV;
      goto out;
   }

   /* release mem region  */
   release_mem_region(res->start, resource_size(res));
   /* dev_err(&pdev->dev, "release io memory region\n"); */

   out:
   return ret;
}

/**
 * @brief EP의 디바이스가 Probe/Resume 되기 위해 필요한 작업을 하는 내부함수
 * @fn      static int _drime4_ep_resume(struct platform_device *pdev, int clk_en)
 * @param   *pdev	[in] platform device 구조체
 * @param   is_probe	[in] probe, resume 프로세스 구분을 위한 변수 (0: resume, 1: probe)
 * @return  실패시 음수값 반환
 *
 * @author  Wooram Son
 */
static int _drime4_ep_resume(struct platform_device *pdev, int is_probe)
{
	int i, ret = 0;
	int ep_core_irq, ep_dma_irq;
	struct drime4_ep *ep = NULL;

	/**
	 * @brief EP Miscellaneous Device Registration
	 */
	ep = kzalloc(sizeof(struct drime4_ep), GFP_KERNEL);
	if (!ep) {
		dev_err(&pdev->dev, "failed to allocate drime4_ep.\n");
		return -EINVAL;
	}

	ep->id = pdev->id;
	ep->dev = &pdev->dev;
	ep->name = pdev->name;

	ep_set_device_info(ep->dev);

	/**
	 * @brief EP Clock Enable
	 */
	if (is_probe == 1) {
		/* Probe */
/*
#ifndef CONFIG_PMU_SELECT
*/
		ep->clock = clk_get(&pdev->dev, "ep");
		if (ep->clock == -2) {
			kfree(ep);
			goto out;
		}
		clk_enable(ep->clock);
		clk_set_rate(ep->clock, 260000000); /* EP Clock freq. */
/*
#endif
*/
	} else {
		/* Resume */
/*
#ifdef CONFIG_PMU_SELECT
*/
		ep->clock = clk_get(&pdev->dev, "ep");
		if (ep->clock == -2) {
			kfree(ep);
			goto out;
		}
		clk_enable(ep->clock);
		clk_set_rate(ep->clock, 260000000);
/*
#endif
*/
	}

	/**
	 * @brief initialize wait_queue
	 */
	ep_top_init_wait_queue();
	ep_path_init();
	ep_path_blocker_init();

	/**
	 * @brief iomap resources 등록
	 */
	for (i = 0; i < 9; i++) {
		ret = drime4_ep_register_iomap(pdev, ep, i);
		if (ret < 0) {
			kfree(ep);
			goto out;
		}
	}

	/**
	 * @brief IRQ 리소스 등록: platform_Get_irq의 파라미터 res_num은 resources 구조체 배열의 인덱스가 아니고 IORESOURCE_IRQ 리소스의 순서를 말함
	 */
	ep_core_irq = platform_get_irq(pdev, 0);
	if (ep_core_irq < 0) {
		dev_err(&pdev->dev, "No IRQ resource!\n");
		ret = -EINVAL;
		kfree(ep);
		goto out;
	}
	if (request_irq(ep_core_irq, drime4_ep_irq, IRQF_DISABLED, EP_MODULE_NAME,
			NULL)) {
		dev_err(&pdev->dev, "failed to request_irq failed (ep_core_irq: %d)\n",
				ep_core_irq);
		ret = -ENODEV;
		kfree(ep);
		goto out;
	}

	ep_dma_irq = platform_get_irq(pdev, 1);
	if (ep_dma_irq < 0) {
		dev_err(&pdev->dev, "No IRQ resource!\n");
		ret = -EINVAL;
		kfree(ep);
		goto out;
	}
	if (request_irq(ep_dma_irq, drime4_ep_irq, IRQF_DISABLED, EP_MODULE_NAME,
			NULL)) {
		dev_err(&pdev->dev, "failed to request_irq failed (ep_dma_irq: %d)\n",
				ep_dma_irq);
		ret = -ENODEV;
		kfree(ep);
		goto out;
	}

	ep->ep_core_irq = ep_core_irq;
	ep->ep_dma_irq = ep_dma_irq;


	if (is_probe) {
		/**
		 * @brief Miscellaneous Device 등록
		 */
		ret = misc_register(&drime4_ep_miscdev);
		if (ret < 0) {
			dev_err(&pdev->dev, "Failed to register misc driver.\n");
			kfree(ep);
			goto out;
		}
		dev_dbg(&pdev->dev, "device registered as /dev/drime4_ep\n");
	}
	
	platform_set_drvdata(pdev, ep);
	ep->dev = &pdev->dev;
	g_ep = ep;

	/**
	 * @brief EP Path Initialization
	 */
	dev_dbg(&pdev->dev, "probe success.\n");

	out:
	return ret;
}

/**
 * @brief EP의 디바이스가 Suspend/Release 되기 위해 필요한 작업을 하는 내부함수
 * @fn      static int _drime4_ep_suspend(struct platform_device *pdev, int clk_di)
 * @param   *pdev	[in] platform device 구조체
 * @param   clk_di	[in] clk disable 여부 (0: don't care, !=0: clk disable)
 * @return  실패시 음수값 반환
 *
 * @author  Wooram Son
 */
static int _drime4_ep_suspend(struct platform_device *pdev, int clk_di)
{
	/**
	 * @brief Miscellaneous Device Deregister
	 */
#ifdef CONFIG_PMU_SELECT
	int reval;
	struct d4_rmu_device *rmu;

	reval = d4_pmu_check(PMU_EP);
	if (reval != 0) {
		reval = ep_pmu_requeset();
		if (reval)
			return -1;

		rmu = d4_rmu_request();
		if (rmu == NULL)
			return;
		d4_pmu_isoen_set(PMU_EP, PMU_CTRL_ON);
		d4_sw_isp_reset(rmu, RST_EP);

		clk_disable(g_ep->clock);
		clk_put(g_ep->clock);

		d4_pmu_scpre_set(PMU_EP, PMU_CTRL_ON);
		d4_pmu_scall_set(PMU_EP, PMU_CTRL_ON);
		d4_rmu_release(rmu);
	}
#endif

	if (clk_di) {
		misc_deregister(&drime4_ep_miscdev);
	}

	iounmap(g_ep->reg_base_top);
	iounmap(g_ep->reg_base_ldc);
	iounmap(g_ep->reg_base_hdr);
	iounmap(g_ep->reg_base_nrm);
	iounmap(g_ep->reg_base_mnf);
	iounmap(g_ep->reg_base_fd);
	iounmap(g_ep->reg_base_bblt);
	iounmap(g_ep->reg_base_lvr);
	iounmap(g_ep->reg_base_dma);

	/**
	 * @brief Release IRQ
	 */
	free_irq(g_ep->ep_core_irq, NULL);
	free_irq(g_ep->ep_dma_irq, NULL);

	/**
	 * @brief EP Clock Disable
	 */
/*
#ifndef CONFIG_PMU_SELECT

	if (clk_di != 0) {
		clk_disable(g_ep->clock);
		clk_put(g_ep->clock);
	}

#endif
*/



	clk_disable(g_ep->clock);
	clk_put(g_ep->clock);

	/**
	 * @brief EP 컨텍스트 메모리 해제
	 */
	kfree(g_ep);

	return 0;
}

/**
 * @brief EP 드라이버 Probe시 호출되는 함수
 * @fn      static int __devinit drime4_ep_probe(struct platform_device *pdev)
 * @param   *pdev	[in] platform device 구조체
 * @return  실패시 음수값 반환
 *
 * @author  Wooram Son
 */
static int __devinit drime4_ep_probe(struct platform_device *pdev)
{
	/**
	 * @brief EP 드라이버 resume 내부 함수를 호출 (Clock 인가)
	 */
	return _drime4_ep_resume(pdev, 1);
}

/**
 * @brief EP 드라이버 Resume시 호출되는 함수
 * @fn      static int drime4_ep_resume(struct platform_device *pdev)
 * @param   *pdev	[in] platform device 구조체
 * @return  실패시 음수값 반환
 *
 * @author  Wooram Son
 */
static int drime4_ep_resume(struct platform_device *pdev)
{
	/**
	 * @brief EP 드라이버 resume 내부 함수를 호출 (Clock 인가 안함)
	 */
	return _drime4_ep_resume(pdev, 0);
}

/**
 * @brief EP 드라이버 Remove시 호출되는 함수
 * @fn      static int drime4_ep_remove(struct platform_device *pdev)
 * @param   *pdev	[in] platform device 구조체
 * @return  실패시 음수값 반환
 *
 * @author  Wooram Son
 */
static int drime4_ep_remove(struct platform_device *pdev)
{
	/**
	 * @brief EP 드라이버 suspend 내부 함수를 호출 (Clock Disable)
	 */
	return _drime4_ep_suspend(pdev, 1);
}

/**
 * @brief EP 드라이버 Suspend시 호출되는 함수
 * @fn      static int drime4_ep_suspend(struct platform_device *pdev, pm_message_t state)
 * @param   *pdev	[in] platform device 구조체
 * @return  실패시 음수값 반환
 *
 * @author  Wooram Son
 */
static int drime4_ep_suspend(struct platform_device *pdev, pm_message_t state)
{
	int i, ret = 0;

	/**
	 * @brief EP 드라이버 suspend 내부 함수를 호출 (Clock Disable 안함)
	 */
	ret = _drime4_ep_suspend(pdev, 0);
	if (ret < 0)
		return ret;

	for (i = 0; i < 9; i++)
		ret = drime4_ep_register_release_iomap(pdev, i);

	return ret;
}

/**
 * @brief EP 플랫폼 디라이버 구조체
 * @author  Wooram Son
 */
static struct platform_driver drime4_ep_driver = {
	.probe = drime4_ep_probe,
	.remove = drime4_ep_remove,
	.suspend = drime4_ep_suspend,
	.resume = drime4_ep_resume,
	.driver = {
		.name = EP_MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

/**
 * @brief EP 드라이버 등록시 호출되는 함수
 * @fn      static int drime4_ep_register(void)
 * @return  0
 *
 * @author  Wooram Son
 */
static int drime4_ep_register(void)
{
	platform_driver_register(&drime4_ep_driver);
	return 0;
}

/**
 * @brief EP 드라이버 unregister시 호출되는 함수
 * @fn      static void drime4_ep_unregister(void)
 * @return  void
 *
 * @author  Wooram Son
 */
static void drime4_ep_unregister(void)
{
	platform_driver_unregister(&drime4_ep_driver);
}

/**
 * @brief EP 드라이버 Register/Unregister 함수 등록
 */
#ifndef CONFIG_SCORE_FAST_RESUME
module_init(drime4_ep_register);
#else
fast_dev_initcall(drime4_ep_register);
#endif
module_exit(drime4_ep_unregister);

MODULE_AUTHOR("Wooram Son<wooram.son@samsung.com>");
MODULE_DESCRIPTION("Samsung Drime IV EP Driver");
MODULE_LICENSE("GPL");
