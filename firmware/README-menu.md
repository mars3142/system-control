# menu.json Reference

The `storage/menu.json` file defines the complete menu structure loaded from SPIFFS at runtime.
It is parsed by the `Mercedes` component and rendered by `Hermes`.

---

## Top-level structure

```json
{
    "screens": [ <Screen>, ... ]
}
```

| Field     | Type            | Required | Description                        |
|-----------|-----------------|----------|------------------------------------|
| `screens` | array of Screen | yes      | List of all screens in the menu    |

The **first** screen in the array becomes the root/home screen.

---

## Screen

```json
{
    "id": "main_menu",
    "title": "Hauptmenü",
    "items": [ <Item>, ... ]
}
```

| Field   | Type           | Required | Description                                  |
|---------|----------------|----------|----------------------------------------------|
| `id`    | string         | yes      | Unique identifier, referenced by `targetScreenId` |
| `title` | string         | yes      | Displayed as the screen heading              |
| `items` | array of Item  | yes      | List of menu items shown on this screen      |

---

## Item (common fields)

Every item shares these base fields:

| Field          | Type    | Required | Description                                                    |
|----------------|---------|----------|----------------------------------------------------------------|
| `id`           | string  | yes      | Unique identifier across all screens                           |
| `type`         | string  | yes      | Item type — see types below                                    |
| `label`        | string  | yes      | Text shown in the menu                                         |
| `persistent`   | bool    | no       | If `true`, value is read/written to NVS and broadcast via WebSocket. Default: `false` |
| `valueType`    | string  | no       | NVS storage type: `"bool"`, `"int"`, or `"string"`. Required when `persistent: true` for toggle and selection items |
| `visibleWhen`  | object  | no       | Conditional visibility — see below                             |

---

## Item types

### `label`

Read-only text. Can display a static string or a dynamic value via the `ItemValueProvider`.

```json
{
    "id": "mac_suffix",
    "type": "label",
    "label": "Device-ID"
}
```

No additional fields. Never persistent.

---

### `action`

Executes an action when SELECT is pressed. Fires `action_manager_execute` with the `actionTopic`.

```json
{
    "id": "ota_update",
    "type": "action",
    "label": "OTA Einspielen",
    "actionTopic": "home/settings/ota_update"
}
```

| Field         | Type   | Required | Description                              |
|---------------|--------|----------|------------------------------------------|
| `actionTopic` | string | yes      | Action key passed to `action_manager_execute` |

---

### `submenu`

Navigates to another screen when SELECT is pressed.

```json
{
    "id": "menu_lights",
    "type": "submenu",
    "label": "Lichtsteuerung",
    "targetScreenId": "lights_menu"
}
```

| Field            | Type   | Required | Description                       |
|------------------|--------|----------|-----------------------------------|
| `targetScreenId` | string | yes      | `id` of the screen to navigate to |

---

### `toggle`

Boolean on/off switch. Toggled with SELECT. Displays an indicator when active.

```json
{
    "id": "light_active",
    "type": "toggle",
    "label": "Einschalten",
    "persistent": true,
    "valueType": "bool",
    "actionTopic": "home/lights/activate"
}
```

| Field         | Type   | Required | Description                                                      |
|---------------|--------|----------|------------------------------------------------------------------|
| `persistent`  | bool   | no       | Saves state to NVS and broadcasts via WebSocket                  |
| `valueType`   | string | yes (if persistent) | Must be `"bool"`                              |
| `actionTopic` | string | no       | If set, fires `action_manager_execute` on change                 |

---

### `selection`

Cycles through a list of options using LEFT/RIGHT. The selected `value` is used for NVS storage.

```json
{
    "id": "light_mode",
    "type": "selection",
    "label": "Modus",
    "persistent": true,
    "valueType": "int",
    "items": [
        { "value": "1", "label": "Tag" },
        { "value": "2", "label": "Nacht" },
        { "value": "0", "label": "Simulation" }
    ]
}
```

| Field        | Type                    | Required | Description                                                      |
|--------------|-------------------------|----------|------------------------------------------------------------------|
| `persistent` | bool                    | no       | Saves selected value to NVS and broadcasts via WebSocket         |
| `valueType`  | string                  | yes (if persistent) | `"int"` or `"string"` depending on NVS storage type |
| `actionTopic`| string                  | no       | If set, fires `action_manager_execute` with the selected value   |
| `items`      | array of SelectionItem  | yes      | At least one entry required                                      |

**SelectionItem:**

| Field   | Type   | Required | Description                                      |
|---------|--------|----------|--------------------------------------------------|
| `value` | string | yes      | Value written to NVS (converted via `valueType`) |
| `label` | string | yes      | Text shown in the menu                           |

The **first** item in the array is the default when no NVS value exists.

---

## `visibleWhen`

Makes an item visible only when another item has a specific value.

```json
{
    "visibleWhen": {
        "itemId": "light_mode",
        "value": "0"
    }
}
```

| Field    | Type   | Required | Description                                              |
|----------|--------|----------|----------------------------------------------------------|
| `itemId` | string | yes      | `id` of the controlling item (must be on any screen)     |
| `value`  | string | yes      | The controlling item's value that makes this item visible |

Works with `toggle` (compare against `"true"`/`"false"`) and `selection` (compare against the selected `value`).
Hidden items are skipped during UP/DOWN navigation.

---

## Complete example

```json
{
    "screens": [
        {
            "id": "main_menu",
            "title": "Hauptmenü",
            "items": [
                {
                    "id": "menu_lights",
                    "type": "submenu",
                    "label": "Lichtsteuerung",
                    "targetScreenId": "lights_menu"
                }
            ]
        },
        {
            "id": "lights_menu",
            "title": "Lichtsteuerung",
            "items": [
                {
                    "id": "light_active",
                    "type": "toggle",
                    "label": "Einschalten",
                    "persistent": true,
                    "valueType": "bool",
                    "actionTopic": "home/lights/activate"
                },
                {
                    "id": "light_mode",
                    "type": "selection",
                    "label": "Modus",
                    "persistent": true,
                    "valueType": "int",
                    "items": [
                        { "value": "1", "label": "Tag" },
                        { "value": "2", "label": "Nacht" },
                        { "value": "0", "label": "Simulation" }
                    ]
                },
                {
                    "id": "light_variant",
                    "type": "selection",
                    "label": "Variante",
                    "persistent": true,
                    "valueType": "int",
                    "visibleWhen": {
                        "itemId": "light_mode",
                        "value": "0"
                    },
                    "items": [
                        { "value": "1", "label": "Standard" },
                        { "value": "2", "label": "Warm" },
                        { "value": "3", "label": "Natur" }
                    ]
                }
            ]
        }
    ]
}
```
