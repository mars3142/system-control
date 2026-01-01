// Device management
function renderDevicesControl() {
    const list = document.getElementById('devices-control-list');
    const noDevices = document.getElementById('no-devices-control');

    list.querySelectorAll('.device-control-item').forEach(el => el.remove());

    if (pairedDevices.length === 0) {
        noDevices.style.display = 'flex';
    } else {
        noDevices.style.display = 'none';
        pairedDevices.forEach(device => {
            const item = document.createElement('div');
            item.className = 'device-control-item';
            const icon = device.type === 'light' ? '💡' : device.type === 'sensor' ? '🌡️' : '📟';
            item.innerHTML = `
                <span class="device-control-icon">${icon}</span>
                <span class="device-control-name">${device.name}</span>
                ${device.type === 'light' ? `<button class="toggle-switch small" onclick="toggleExternalDevice('${device.id}')"><span class="toggle-icon">💡</span></button>` : ''}
            `;
            list.insertBefore(item, noDevices);
        });
    }
}

async function toggleExternalDevice(deviceId) {
    try {
        await fetch('/api/devices/toggle', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id: deviceId })
        });
    } catch (error) {
        console.log('Demo: Gerät umgeschaltet');
    }
}

async function scanDevices() {
    const loading = document.getElementById('devices-loading');
    const unpairedList = document.getElementById('unpaired-devices');
    const noDevices = document.getElementById('no-unpaired-devices');

    loading.classList.add('active');
    noDevices.style.display = 'none';

    // Entferne vorherige Ergebnisse (außer empty-state)
    unpairedList.querySelectorAll('.device-item').forEach(el => el.remove());

    try {
        const response = await fetch('/api/devices/scan');
        const devices = await response.json();

        loading.classList.remove('active');

        if (devices.length === 0) {
            noDevices.style.display = 'flex';
        } else {
            devices.forEach(device => {
                const item = createUnpairedDeviceItem(device);
                unpairedList.insertBefore(item, noDevices);
            });
        }

        showStatus('devices-status', t('devices.found', { count: devices.length }), 'success');
    } catch (error) {
        loading.classList.remove('active');
        // Demo data
        const demoDevices = [
            { id: 'matter-001', type: 'light', name: 'Matter Lamp' },
            { id: 'matter-002', type: 'sensor', name: 'Temperature Sensor' }
        ];

        demoDevices.forEach(device => {
            const item = createUnpairedDeviceItem(device);
            unpairedList.insertBefore(item, noDevices);
        });

        showStatus('devices-status', `Demo: ${t('devices.found', { count: 2 })}`, 'success');
    }
}

function createUnpairedDeviceItem(device) {
    const item = document.createElement('div');
    item.className = 'device-item unpaired';
    item.dataset.id = device.id;

    const icon = device.type === 'light' ? '💡' : device.type === 'sensor' ? '🌡️' : '📟';
    const unknownDevice = getCurrentLanguage() === 'en' ? 'Unknown Device' : 'Unbekanntes Gerät';

    item.innerHTML = `
        <div class="device-info">
            <span class="device-icon">${icon}</span>
            <div class="device-details">
                <span class="device-name">${device.name || unknownDevice}</span>
                <span class="device-id">${device.id}</span>
            </div>
        </div>
        <button class="btn btn-primary btn-small" onclick="pairDevice('${device.id}', '${device.name || unknownDevice}', '${device.type || 'unknown'}')">
            ➕ ${t('btn.add')}
        </button>
    `;

    return item;
}

async function pairDevice(id, name, type) {
    try {
        const response = await fetch('/api/devices/pair', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id, name })
        });

        if (response.ok) {
            showStatus('devices-status', t('devices.added', { name }), 'success');
            // Entferne aus unpaired Liste
            document.querySelector(`.device-item[data-id="${id}"]`)?.remove();
            // Lade paired Geräte neu
            loadPairedDevices();
        } else {
            throw new Error(t('error'));
        }
    } catch (error) {
        // Demo mode
        showStatus('devices-status', `Demo: ${t('devices.added', { name })}`, 'success');
        document.querySelector(`.device-item.unpaired[data-id="${id}"]`)?.remove();

        // Füge zu Demo-Liste hinzu
        pairedDevices.push({ id, name, type });
        renderPairedDevices();
    }
}

async function loadPairedDevices() {
    try {
        const response = await fetch('/api/devices/paired');
        pairedDevices = await response.json();
        renderPairedDevices();
    } catch (error) {
        // Keep demo data
        renderPairedDevices();
    }
}

function renderPairedDevices() {
    const list = document.getElementById('paired-devices');
    const noDevices = document.getElementById('no-paired-devices');

    // Remove previous entries
    list.querySelectorAll('.device-item').forEach(el => el.remove());

    if (pairedDevices.length === 0) {
        noDevices.style.display = 'flex';
    } else {
        noDevices.style.display = 'none';
        pairedDevices.forEach(device => {
            const item = createPairedDeviceItem(device);
            list.insertBefore(item, noDevices);
        });
    }

    // Also update the control page
    renderDevicesControl();
}

function createPairedDeviceItem(device) {
    const item = document.createElement('div');
    item.className = 'device-item paired';
    item.dataset.id = device.id;

    const icon = device.type === 'light' ? '💡' : device.type === 'sensor' ? '🌡️' : '📟';
    const placeholder = getCurrentLanguage() === 'en' ? 'Device name' : 'Gerätename';

    item.innerHTML = `
        <div class="device-info">
            <span class="device-icon">${icon}</span>
            <div class="device-details">
                <input type="text" class="device-name-input" value="${device.name}" 
                       onchange="updateDeviceName('${device.id}', this.value)"
                       placeholder="${placeholder}">
                <span class="device-id">${device.id}</span>
            </div>
        </div>
        <button class="btn btn-secondary btn-small btn-danger" onclick="unpairDevice('${device.id}', '${device.name}')">
            🗑️
        </button>
    `;

    return item;
}

async function updateDeviceName(id, newName) {
    try {
        const response = await fetch('/api/devices/update', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id, name: newName })
        });

        if (response.ok) {
            showStatus('devices-status', t('devices.name.updated'), 'success');
        }
    } catch (error) {
        // Demo mode - update locally
        const device = pairedDevices.find(d => d.id === id);
        if (device) device.name = newName;
        showStatus('devices-status', `Demo: ${t('devices.name.updated')}`, 'success');
    }
}

async function unpairDevice(id, name) {
    if (!confirm(t('devices.confirm.remove', { name }))) return;

    try {
        const response = await fetch('/api/devices/unpair', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id })
        });

        if (response.ok) {
            showStatus('devices-status', t('devices.removed', { name }), 'success');
            loadPairedDevices();
        }
    } catch (error) {
        // Demo mode
        pairedDevices = pairedDevices.filter(d => d.id !== id);
        renderPairedDevices();
        showStatus('devices-status', `Demo: ${t('devices.removed', { name })}`, 'success');
    }
}
