// Scene functions
async function loadScenes() {
    try {
        const response = await fetch('/api/scenes');
        scenes = await response.json();
    } catch (error) {
        // Demo data
        scenes = [
            { id: 'scene-1', name: 'Abendstimmung', icon: '🌅', actions: { light: 'on', mode: 'simulation', schema: 'schema_02.csv' } },
            { id: 'scene-2', name: 'Nachtmodus', icon: '🌙', actions: { light: 'on', mode: 'night' } }
        ];
    }
    renderScenesConfig();
    renderScenesControl();
}

function renderScenesConfig() {
    const list = document.getElementById('scenes-config-list');
    const noScenes = document.getElementById('no-scenes-config');

    list.querySelectorAll('.scene-config-item').forEach(el => el.remove());

    if (scenes.length === 0) {
        noScenes.style.display = 'flex';
    } else {
        noScenes.style.display = 'none';
        scenes.forEach(scene => {
            const item = createSceneConfigItem(scene);
            list.insertBefore(item, noScenes);
        });
    }
}

function createSceneConfigItem(scene) {
    const item = document.createElement('div');
    item.className = 'scene-config-item';
    item.dataset.id = scene.id;

    const actionsText = [];
    if (scene.actions.light) actionsText.push(`${t('control.light.light')} ${scene.actions.light === 'on' ? t('common.on') : t('common.off')}`);
    if (scene.actions.mode) actionsText.push(`${t('control.status.mode')}: ${t('mode.' + scene.actions.mode)}`);
    if (scene.actions.schema) actionsText.push(`${t('control.status.schema')}: ${scene.actions.schema.replace('.csv', '')}`);
    if (scene.actions.devices && scene.actions.devices.length > 0) {
        actionsText.push(t('devices.found', { count: scene.actions.devices.length }));
    }

    item.innerHTML = `
        <div class="scene-info">
            <span class="scene-icon">${scene.icon}</span>
            <div class="scene-details">
                <span class="scene-name">${scene.name}</span>
                <span class="scene-actions-text">${actionsText.join(', ')}</span>
            </div>
        </div>
        <div class="scene-buttons">
            <button class="btn btn-secondary btn-small" onclick="editScene('${scene.id}')">✏️</button>
            <button class="btn btn-secondary btn-small btn-danger" onclick="deleteScene('${scene.id}', '${scene.name}')">🗑️</button>
        </div>
    `;

    return item;
}

function renderScenesControl() {
    const list = document.getElementById('scenes-control-list');
    const noScenes = document.getElementById('no-scenes-control');

    list.querySelectorAll('.scene-btn').forEach(el => el.remove());

    if (scenes.length === 0) {
        noScenes.style.display = 'flex';
    } else {
        noScenes.style.display = 'none';
        scenes.forEach(scene => {
            const btn = document.createElement('button');
            btn.className = 'scene-btn';
            btn.onclick = () => activateScene(scene.id);
            btn.innerHTML = `
                <span class="scene-btn-icon">${scene.icon}</span>
                <span class="scene-btn-name">${scene.name}</span>
            `;
            list.insertBefore(btn, noScenes);
        });
    }
}

function openSceneModal() {
    currentEditScene = null;
    selectedSceneIcon = '🌅';
    document.getElementById('scene-modal-title').textContent = t('modal.scene.new');
    document.getElementById('scene-name').value = '';
    document.getElementById('scene-action-light').checked = true;
    document.getElementById('scene-light-state').value = 'on';
    document.getElementById('scene-action-mode').checked = false;
    document.getElementById('scene-mode-value').value = 'simulation';
    document.getElementById('scene-action-schema').checked = false;
    document.getElementById('scene-schema-value').value = 'schema_01.csv';

    document.querySelectorAll('.icon-btn').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.icon === '🌅');
    });

    renderSceneDevicesList();

    document.getElementById('scene-modal').classList.add('active');
    document.body.style.overflow = 'hidden';
}

function editScene(sceneId) {
    const scene = scenes.find(s => s.id === sceneId);
    if (!scene) return;

    currentEditScene = sceneId;
    selectedSceneIcon = scene.icon;
    document.getElementById('scene-modal-title').textContent = t('modal.scene.edit');
    document.getElementById('scene-name').value = scene.name;

    document.getElementById('scene-action-light').checked = !!scene.actions.light;
    document.getElementById('scene-light-state').value = scene.actions.light || 'on';
    document.getElementById('scene-action-mode').checked = !!scene.actions.mode;
    document.getElementById('scene-mode-value').value = scene.actions.mode || 'simulation';
    document.getElementById('scene-action-schema').checked = !!scene.actions.schema;
    document.getElementById('scene-schema-value').value = scene.actions.schema || 'schema_01.csv';

    document.querySelectorAll('.icon-btn').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.icon === scene.icon);
    });

    renderSceneDevicesList(scene.actions.devices || []);

    document.getElementById('scene-modal').classList.add('active');
    document.body.style.overflow = 'hidden';
}

// Render device list in scene modal
function renderSceneDevicesList(selectedDevices = []) {
    const list = document.getElementById('scene-devices-list');
    const noDevices = document.getElementById('no-scene-devices');

    // Remove previous entries (except empty-state)
    list.querySelectorAll('.scene-device-item').forEach(el => el.remove());

    if (pairedDevices.length === 0) {
        noDevices.style.display = 'flex';
    } else {
        noDevices.style.display = 'none';
        pairedDevices.forEach(device => {
            const selectedDevice = selectedDevices.find(d => d.id === device.id);
            const isSelected = !!selectedDevice;
            const deviceState = selectedDevice ? selectedDevice.state : 'on';

            const item = document.createElement('div');
            item.className = 'scene-device-item';
            item.dataset.id = device.id;

            const icon = device.type === 'light' ? '💡' : device.type === 'sensor' ? '🌡️' : '📟';

            item.innerHTML = `
                <label class="scene-device-checkbox">
                    <input type="checkbox" ${isSelected ? 'checked' : ''} 
                           onchange="toggleSceneDevice('${device.id}')">
                    <span class="device-icon">${icon}</span>
                    <span class="device-name">${device.name}</span>
                </label>
                ${device.type === 'light' ? `
                <select class="scene-device-state" id="scene-device-state-${device.id}" 
                        ${!isSelected ? 'disabled' : ''}>
                    <option value="on" ${deviceState === 'on' ? 'selected' : ''}>${t('scene.light.on')}</option>
                    <option value="off" ${deviceState === 'off' ? 'selected' : ''}>${t('scene.light.off')}</option>
                </select>
                ` : ''}
            `;

            list.insertBefore(item, noDevices);
        });
    }
}

function toggleSceneDevice(deviceId) {
    const stateSelect = document.getElementById(`scene-device-state-${deviceId}`);
    if (stateSelect) {
        const checkbox = document.querySelector(`.scene-device-item[data-id="${deviceId}"] input[type="checkbox"]`);
        stateSelect.disabled = !checkbox.checked;
    }
}

function getSelectedSceneDevices() {
    const devices = [];
    document.querySelectorAll('.scene-device-item').forEach(item => {
        const checkbox = item.querySelector('input[type="checkbox"]');
        if (checkbox && checkbox.checked) {
            const deviceId = item.dataset.id;
            const stateSelect = document.getElementById(`scene-device-state-${deviceId}`);
            devices.push({
                id: deviceId,
                state: stateSelect ? stateSelect.value : 'on'
            });
        }
    });
    return devices;
}

function closeSceneModal() {
    document.getElementById('scene-modal').classList.remove('active');
    document.body.style.overflow = '';
    currentEditScene = null;
}

function selectSceneIcon(icon) {
    selectedSceneIcon = icon;
    document.querySelectorAll('.icon-btn').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.icon === icon);
    });
}

async function saveScene() {
    const name = document.getElementById('scene-name').value.trim();
    if (!name) {
        showStatus('scenes-status', t('scenes.error.name'), 'error');
        return;
    }

    const actions = {};
    if (document.getElementById('scene-action-light').checked) {
        actions.light = document.getElementById('scene-light-state').value;
    }
    if (document.getElementById('scene-action-mode').checked) {
        actions.mode = document.getElementById('scene-mode-value').value;
    }
    if (document.getElementById('scene-action-schema').checked) {
        actions.schema = document.getElementById('scene-schema-value').value;
    }

    // Add device actions
    const selectedDevices = getSelectedSceneDevices();
    if (selectedDevices.length > 0) {
        actions.devices = selectedDevices;
    }

    const sceneData = {
        id: currentEditScene || `scene-${Date.now()}`,
        name,
        icon: selectedSceneIcon,
        actions
    };

    try {
        const response = await fetch('/api/scenes', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(sceneData)
        });

        if (response.ok) {
            showStatus('scenes-status', currentEditScene ? t('scenes.updated') : t('scenes.created'), 'success');
            loadScenes();
            closeSceneModal();
        }
    } catch (error) {
        // Demo mode
        if (currentEditScene) {
            const index = scenes.findIndex(s => s.id === currentEditScene);
            if (index !== -1) scenes[index] = sceneData;
        } else {
            scenes.push(sceneData);
        }
        renderScenesConfig();
        renderScenesControl();
        showStatus('scenes-status', `Demo: ${currentEditScene ? t('scenes.updated') : t('scenes.created')}`, 'success');
        closeSceneModal();
    }
}

async function deleteScene(sceneId, name) {
    if (!confirm(t('scenes.confirm.delete', { name }))) return;

    try {
        const response = await fetch('/api/scenes', {
            method: 'DELETE',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id: sceneId })
        });

        if (response.ok) {
            showStatus('scenes-status', t('scenes.deleted', { name }), 'success');
            loadScenes();
        }
    } catch (error) {
        // Demo mode
        scenes = scenes.filter(s => s.id !== sceneId);
        renderScenesConfig();
        renderScenesControl();
        showStatus('scenes-status', `Demo: ${t('scenes.deleted', { name })}`, 'success');
    }
}

async function activateScene(sceneId) {
    const scene = scenes.find(s => s.id === sceneId);
    if (!scene) return;

    try {
        await fetch('/api/scenes/activate', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id: sceneId })
        });
        showStatus('scenes-control-status', t('scenes.activated', { name: scene.name }), 'success');
    } catch (error) {
        // Demo: Execute actions
        if (scene.actions.light === 'on') {
            lightOn = true;
            updateLightToggle();
        } else if (scene.actions.light === 'off') {
            lightOn = false;
            updateLightToggle();
        }
        if (scene.actions.mode) {
            currentMode = scene.actions.mode;
            updateModeButtons();
            updateSimulationOptions();
        }
        // Device actions in demo mode
        if (scene.actions.devices && scene.actions.devices.length > 0) {
            scene.actions.devices.forEach(deviceAction => {
                console.log(`Demo: Device ${deviceAction.id} -> ${deviceAction.state}`);
            });
        }
        showStatus('scenes-control-status', `Demo: ${t('scenes.activated', { name: scene.name })}`, 'success');
    }
}
