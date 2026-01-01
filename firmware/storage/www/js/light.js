// Light control
async function toggleLight() {
    lightOn = !lightOn;
    updateLightToggle();

    try {
        const response = await fetch('/api/light/power', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ on: lightOn })
        });

        if (response.ok) {
            showStatus('light-status', `${t('control.light.light')} ${lightOn ? t('common.on') : t('common.off')}`, 'success');
        } else {
            throw new Error(t('error'));
        }
    } catch (error) {
        showStatus('light-status', `Demo: ${t('control.light.light')} ${lightOn ? t('common.on') : t('common.off')}`, 'success');
    }
}

function updateLightToggle() {
    const toggle = document.getElementById('light-toggle');
    const state = document.getElementById('light-state');
    const icon = document.getElementById('light-icon');

    if (lightOn) {
        toggle.classList.add('active');
        state.textContent = t('common.on');
        icon.textContent = '💡';
    } else {
        toggle.classList.remove('active');
        state.textContent = t('common.off');
        icon.textContent = '💡';
    }
}

// Mode control
async function setMode(mode) {
    currentMode = mode;
    updateModeButtons();
    updateSimulationOptions();

    try {
        const response = await fetch('/api/light/mode', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ mode })
        });

        if (response.ok) {
            const modeName = t(`mode.${mode}`);
            showStatus('mode-status', `${t('control.status.mode')}: "${modeName}"`, 'success');
            document.getElementById('current-mode').textContent = modeName;
        } else {
            throw new Error(t('error'));
        }
    } catch (error) {
        const modeName = t(`mode.${mode}`);
        showStatus('mode-status', `Demo: ${t('control.status.mode')} "${modeName}"`, 'success');
        document.getElementById('current-mode').textContent = modeName;
    }
}

function updateModeButtons() {
    document.querySelectorAll('.mode-btn').forEach(btn => btn.classList.remove('active'));
    document.getElementById(`mode-${currentMode}`).classList.add('active');
}

function updateSimulationOptions() {
    const options = document.getElementById('simulation-options');
    if (currentMode === 'simulation') {
        options.classList.add('visible');
    } else {
        options.classList.remove('visible');
    }
}

async function setActiveSchema() {
    const schema = document.getElementById('active-schema').value;
    const schemaNum = schema.replace('schema_0', '').replace('.csv', '');
    const schemaName = t(`schema.name.${schemaNum}`);

    try {
        const response = await fetch('/api/light/schema', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ schema })
        });

        if (response.ok) {
            showStatus('mode-status', `${t('control.status.schema')}: "${schemaName}"`, 'success');
            document.getElementById('current-schema').textContent = schemaName;
        } else {
            throw new Error(t('error'));
        }
    } catch (error) {
        showStatus('mode-status', `Demo: ${schemaName}`, 'success');
        document.getElementById('current-schema').textContent = schemaName;
    }
}
