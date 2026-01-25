// Light control
let thunderOn = false;

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

// Thunder control
async function toggleThunder() {
    thunderOn = !thunderOn;
    updateThunderToggle();

    try {
        const response = await fetch('/api/light/thunder', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ on: thunderOn })
        });

        if (response.ok) {
            showStatus('light-status', `${t('control.light.thunder')} ${thunderOn ? t('common.on') : t('common.off')}`, 'success');
        } else {
            throw new Error(t('error'));
        }
    } catch (error) {
        showStatus('light-status', `Demo: ${t('control.light.thunder')} ${thunderOn ? t('common.on') : t('common.off')}`, 'success');
    }
}

function updateThunderToggle() {
    const toggle = document.getElementById('thunder-toggle');
    const state = document.getElementById('thunder-state');
    const icon = document.getElementById('thunder-icon');

    if (thunderOn) {
        toggle.classList.add('active');
        state.textContent = t('common.on');
        icon.textContent = '⛈️';
    } else {
        toggle.classList.remove('active');
        state.textContent = t('common.off');
        icon.textContent = '⚡';
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

    [
        'control.status.clock'
    ].forEach(i18nKey => {
        const label = document.querySelector(`.status-item .status-label[data-i18n="${i18nKey}"]`);
        const item = label ? label.closest('.status-item') : null;
        if (item) {
            if (currentMode === 'simulation') {
                item.classList.add('visible');
            } else {
                item.classList.remove('visible');
            }
        }
    });
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

/**
 * Load light status from server
 */
async function loadLightStatus() {
    try {
        const response = await fetch('/api/light/status');
        if (response.ok) {
            const status = await response.json();

            // Update light state
            if (typeof status.on === 'boolean') {
                lightOn = status.on;
                updateLightToggle();
            }

            // Update thunder state
            if (typeof status.thunder === 'boolean') {
                thunderOn = status.thunder;
                updateThunderToggle();
            }

            // Update mode
            if (status.mode) {
                currentMode = status.mode;
                updateModeButtons();
                updateSimulationOptions();
                document.getElementById('current-mode').textContent = t(`mode.${status.mode}`);
            }

            // Update schema
            if (status.schema) {
                document.getElementById('active-schema').value = status.schema;
            }

            // Update current color
            if (status.color) {
                const colorPreview = document.getElementById('current-color');
                if (colorPreview) {
                    colorPreview.style.backgroundColor = `rgb(${status.color.r}, ${status.color.g}, ${status.color.b})`;
                }
            }

            // Update clock/time
            if (status.clock) {
                const clockEl = document.getElementById('current-clock');
                if (clockEl) {
                    // Use one translation key for the suffix, language is handled by t()
                    clockEl.textContent = status.clock + ' ' + t('clock.suffix');
                }
            }
        }
    } catch (error) {
        console.log('Light status not available');
    }
}
