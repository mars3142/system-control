#include "ui/ExternalDevices.h"

#include "ble/ble_device.h"
#include "ble/ble_scanner.h"
#include "ble_manager.h"

ExternalDevices::ExternalDevices(menu_options_t *options) : Menu(options), m_options(options)
{
    for (int i = 0; i < get_device_count(); i++)
    {
        device_info_t *device = get_device(i);
        if (device != nullptr)
        {
            addText(i, device->name);
        }
    }
}

void ExternalDevices::onButtonPressed(const MenuItem &menuItem, const ButtonType button)
{
    if (button == ButtonType::SELECT)
    {
        ble_connect_to_device(menuItem.getId());
    }
}

IMPLEMENT_GET_NAME(ExternalDevices)
