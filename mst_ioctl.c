#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/uaccess.h>
#include "mst_dma.h"
#include "mst_defs.h"
#include "mst_pci_conf_access.h"


int dma_pages_ioctl(unsigned int command, void* user_buffer,
                    struct mst_device* mst_device)
{
    struct page_info page_info;
    int error_code = 0;


    /* Copy the page info structure from user space. */
    if (copy_from_user(&page_info, user_buffer,
                       sizeof(struct page_info))) {
        error_code = -EFAULT;
        goto ReturnOnFinished;
    }

    if (command == MST_GET_DMA_PAGES) {
        if (map_dma_pages(&page_info, mst_device)) {
            goto ReturnOnFinished;
        }

        /* Return the physical address to the user */
        if (copy_to_user(user_buffer, &page_info,
                         sizeof(struct page_info)) != 0) {
            error_code = -EFAULT;
            goto ReturnOnFinished;
        }
    } else {
        error_code =
            release_dma_pages(&page_info, mst_device);
    }

ReturnOnFinished:
    return error_code;
}



int read_dword_ioctl(unsigned int command, void* user_buffer,
                     struct mst_device* mst_device)
{
    struct read_dword_from_config_space read_from_cspace;
    int error_code = 0;


    /* Copy the request from user space. */
    if (copy_from_user(&read_from_cspace, user_buffer,
                sizeof(struct read_dword_from_config_space)) != 0) {
        error_code = -EFAULT;
        goto ReturnOnFinished;
    }

    /* Read the dword. */
    if (read_dword(&read_from_cspace, mst_device)) {
        goto ReturnOnFinished;
    }

    /* Copy the data to the user space. */
    if (copy_to_user(user_buffer, &read_from_cspace,
                sizeof(struct read_dword_from_config_space)) != 0) {
        error_code = -EFAULT;
        goto ReturnOnFinished;
    }

ReturnOnFinished:
    return error_code;
}


int get_mst_device_parameters(unsigned int command, void* user_buffer,
                              struct mst_device* mst_device)
{
    struct device_parameters parameters;
    int error = 0;

    /* Copy the request from user space. */
    if (copy_from_user(&parameters, user_buffer,
                        sizeof(struct device_parameters)) != 0) {
            error = -EFAULT;
            goto ReturnOnFinished;
    }

    parameters.domain = pci_domain_nr(mst_device->pci_device->bus);
    parameters.bus = mst_device->pci_device->bus->number;
    parameters.slot = PCI_SLOT(mst_device->pci_device->devfn);
    parameters.func = PCI_FUNC(mst_device->pci_device->devfn);
    parameters.pci_memory_bar_address= mst_device->memory_device.pci_memory_bar_address;
    parameters.device = mst_device->pci_device->device;
    parameters.vendor = mst_device->pci_device->vendor;
    parameters.subsystem_device = mst_device->pci_device->subsystem_device;
    parameters.subsystem_vendor = mst_device->pci_device->subsystem_vendor;

    check_address_space_support(mst_device);
    if (mst_device->pciconf_device.vendor_specific_capability &&
            (mst_device->pciconf_device.address_space.icmd|| mst_device->pciconf_device.address_space.cr_space ||
             mst_device->pciconf_device.address_space.semaphore)) {
        parameters.vendor_specific_capability = mst_device->pciconf_device.vendor_specific_capability;
    } else {
        parameters.vendor_specific_capability = 0;
    }

    /* Copy the data to the user space. */
    if (copy_to_user(user_buffer, &parameters,
                     sizeof(struct device_parameters)) != 0) {
            error = -EFAULT;
            goto ReturnOnFinished;
    }

ReturnOnFinished:
    return error;
}


int pci_connectx_wa(unsigned int command, void* user_buffer,
                    struct mst_device* mst_device)
{
    unsigned int slot_mask;
    struct mst_connectx_wa connectx_wa;
    int error = 0;

    /* Is this slot exists ? */
    if (mst_device->memory_device.connectx_wa_slot_p1) {
            mst_debug("Slot exits for file %s, slot:0x%x\n",
                      mst_device->device_name, mst_device->memory_device.connectx_wa_slot_p1);
            error = 0;
            goto ReturnOnFinished;
    }

    /* Find first un(set) bit. and remember the slot */
    mst_device->memory_device.connectx_wa_slot_p1= ffs(~mst_device->memory_device.connectx_wa_slot_p1);
    if (mst_device->memory_device.connectx_wa_slot_p1 == 0 ||
            mst_device->memory_device.connectx_wa_slot_p1 > MST_CONNECTX_WA_SIZE) {
            error = -ENOLCK;
            goto ReturnOnFinished;
    }

    slot_mask = 1 << (mst_device->memory_device.connectx_wa_slot_p1 - 1);

    /* set the slot as taken */
    mst_device->memory_device.connectx_wa_slot_p1 |= slot_mask;

    connectx_wa.connectx_wa_slot_p1 = mst_device->memory_device.connectx_wa_slot_p1;

    /* Copy the data to the user space. */
    if (copy_to_user(user_buffer, &connectx_wa,
                     sizeof(struct mst_connectx_wa)) != 0) {
            error = -EFAULT;
            goto ReturnOnFinished;
    }

ReturnOnFinished:
    return error;
}