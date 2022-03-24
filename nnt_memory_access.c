#include "nnt_device_defs.h"
#include "nnt_pci_conf_access_defs.h"
#include "nnt_pci_conf_access.h"
#include "nnt_defs.h"


int write_memory(struct nnt_device* nnt_device, struct rw_operation* write_operation)
{
    /* Endianness conversion. */
    cpu_to_be32s(write_operation->data);
    write_operation->data[0] = cpu_to_le32(write_operation->data[0]);

    /* Write to the hardware memory address. */
    iowrite32(write_operation->data[0], nnt_device->memory_device.hardware_memory_address + write_operation->offset);

    return 0;
}



int read_memory(struct nnt_device* nnt_device, struct rw_operation* read_operation)
{
    /* Read from the hardware memory address. */
    read_operation->data[0] = ioread32(nnt_device->memory_device.hardware_memory_address + read_operation->offset);

    /* Endianness conversion */
    be32_to_cpus(read_operation->data);
    read_operation->data[0] = cpu_to_le32(read_operation->data[0]);

    return 0;
}


int init_memory(unsigned int command, void* user_buffer,
                struct nnt_device* nnt_device)
{
    return 0;
}