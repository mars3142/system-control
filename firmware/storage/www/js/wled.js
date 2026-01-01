// LED Configuration Module
// Manages LED segments and configuration

let wledConfig = {
    segments: []
};

/**
 * Initialize WLED module
 */
function initWled() {
    loadWledConfig();
}

/**
 * Load WLED configuration from server
 */
async function loadWledConfig() {
    try {
        const response = await fetch('/api/wled/config');
        if (response.ok) {
            wledConfig = await response.json();
            renderWledSegments();
            showStatus('wled-status', t('wled.loaded'), 'success');
        }
    } catch (error) {
        console.log('Using default LED config');
        wledConfig = { segments: [] };
        renderWledSegments();
    }
}

/**
 * Render WLED segments list
 */
function renderWledSegments() {
    const list = document.getElementById('wled-segments-list');
    const emptyState = document.getElementById('no-wled-segments');

    if (!list) return;

    // Clear existing segments (keep empty state)
    const existingItems = list.querySelectorAll('.wled-segment-item');
    existingItems.forEach(item => item.remove());

    if (wledConfig.segments.length === 0) {
        if (emptyState) emptyState.style.display = 'block';
        return;
    }

    if (emptyState) emptyState.style.display = 'none';

    wledConfig.segments.forEach((segment, index) => {
        const item = createSegmentElement(segment, index);
        list.insertBefore(item, emptyState);
    });
}

/**
 * Create segment DOM element
 * @param {object} segment - Segment data
 * @param {number} index - Segment index
 * @returns {HTMLElement} Segment element
 */
function createSegmentElement(segment, index) {
    const item = document.createElement('div');
    item.className = 'wled-segment-item';
    item.dataset.index = index;

    item.innerHTML = `
        <div class="segment-name-field">
            <label class="segment-number">${t('wled.segment.name', { num: index + 1 })}</label>
            <input type="text" class="segment-name-input" value="${escapeHtml(segment.name || '')}" 
                placeholder="${t('wled.segment.name', { num: index + 1 })}"
                onchange="updateSegment(${index}, 'name', this.value)">
        </div>
        <div class="segment-field">
            <label data-i18n="wled.segment.start">${t('wled.segment.start')}</label>
            <input type="number" min="0" value="${segment.start || 0}" 
                onchange="updateSegment(${index}, 'start', parseInt(this.value))">
        </div>
        <div class="segment-field">
            <label data-i18n="wled.segment.leds">${t('wled.segment.leds')}</label>
            <input type="number" min="1" value="${segment.leds || 1}" 
                onchange="updateSegment(${index}, 'leds', parseInt(this.value))">
        </div>
        <button class="segment-remove-btn" onclick="removeWledSegment(${index})" title="${t('wled.segment.remove')}">
            🗑️
        </button>
    `;

    return item;
}

/**
 * Add a new WLED segment
 */
function addWledSegment() {
    // Calculate next start position
    let nextStart = 0;
    if (wledConfig.segments.length > 0) {
        const lastSegment = wledConfig.segments[wledConfig.segments.length - 1];
        nextStart = (lastSegment.start || 0) + (lastSegment.leds || 0);
    }

    wledConfig.segments.push({
        name: '',
        start: nextStart,
        leds: 10
    });

    renderWledSegments();
}

/**
 * Update a segment property
 * @param {number} index - Segment index
 * @param {string} property - Property name
 * @param {*} value - New value
 */
function updateSegment(index, property, value) {
    if (index >= 0 && index < wledConfig.segments.length) {
        wledConfig.segments[index][property] = value;
    }
}

/**
 * Remove a WLED segment
 * @param {number} index - Segment index to remove
 */
function removeWledSegment(index) {
    if (index >= 0 && index < wledConfig.segments.length) {
        wledConfig.segments.splice(index, 1);
        renderWledSegments();
    }
}

/**
 * Save WLED configuration to server
 */
async function saveWledConfig() {
    try {
        const response = await fetch('/api/wled/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(wledConfig)
        });

        if (response.ok) {
            showStatus('wled-status', t('wled.saved'), 'success');
        } else {
            throw new Error('Save failed');
        }
    } catch (error) {
        console.error('Error saving WLED config:', error);
        showStatus('wled-status', t('wled.error.save'), 'error');
    }
}

/**
 * Helper function to escape HTML
 * @param {string} text - Text to escape
 * @returns {string} Escaped text
 */
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', initWled);
