// Schema functions
async function loadSchema() {
    const schemaFile = document.getElementById('schema-select').value;
    const grid = document.getElementById('schema-grid');
    const loading = document.getElementById('schema-loading');

    grid.innerHTML = '';
    loading.classList.add('active');

    try {
        const response = await fetch(`/api/schema/${schemaFile}`);
        const text = await response.text();

        schemaData = parseCSV(text);
        renderSchemaGrid();
        showStatus('schema-status', t('schema.loaded', { file: schemaFile }), 'success');
    } catch (error) {
        // Demo data for local testing
        schemaData = generateDemoData();
        renderSchemaGrid();
        showStatus('schema-status', t('schema.demo'), 'error');
    } finally {
        loading.classList.remove('active');
    }
}

function parseCSV(text) {
    const lines = text.trim().split('\n');
    return lines
        .filter(line => line.trim() && !line.startsWith('#'))
        .map(line => {
            const values = line.split(',').map(v => parseInt(v.trim()));
            return {
                r: values[0] || 0,
                g: values[1] || 0,
                b: values[2] || 0,
                v1: values[3] || 0,
                v2: values[4] || 0,
                v3: values[5] || 250
            };
        });
}

function generateDemoData() {
    const data = [];
    for (let i = 0; i < 48; i++) {
        const hour = i / 2;
        let r, g, b;

        if (hour < 6 || hour >= 22) {
            r = 25; g = 25; b = 112;
        } else if (hour < 8) {
            const t = (hour - 6) / 2;
            r = Math.round(25 + 230 * t);
            g = Math.round(25 + 150 * t);
            b = Math.round(112 + 50 * t);
        } else if (hour < 18) {
            r = 255; g = 240; b = 220;
        } else {
            const t = (hour - 18) / 4;
            r = Math.round(255 - 230 * t);
            g = Math.round(240 - 215 * t);
            b = Math.round(220 - 108 * t);
        }

        data.push({
            r, g, b,
            v1: 0,
            v2: Math.round(100 + 155 * Math.sin(Math.PI * hour / 12)),
            v3: 250
        });
    }
    return data;
}

function renderSchemaGrid() {
    const grid = document.getElementById('schema-grid');
    grid.innerHTML = '';

    for (let i = 0; i < 48; i++) {
        const hour = Math.floor(i / 2);
        const minute = (i % 2) * 30;
        const time = `${hour.toString().padStart(2, '0')}:${minute.toString().padStart(2, '0')}`;

        const data = schemaData[i] || { r: 0, g: 0, b: 0, v1: 0, v2: 100, v3: 250 };

        const row = document.createElement('div');
        row.className = 'time-row';
        row.dataset.index = i;

        row.innerHTML = `
            <span class="time-label">${time}</span>
            <div class="color-preview" 
                 style="background: rgb(${data.r}, ${data.g}, ${data.b})"
                 onclick="openColorModal(${i})"
                 title="Tippen zum Bearbeiten"></div>
            <input type="number" class="value-input" inputmode="numeric" pattern="[0-9]*" min="0" max="255" value="${data.r}" 
                   onchange="updateValue(${i}, 'r', this.value)">
            <input type="number" class="value-input" inputmode="numeric" pattern="[0-9]*" min="0" max="255" value="${data.g}" 
                   onchange="updateValue(${i}, 'g', this.value)">
            <input type="number" class="value-input" inputmode="numeric" pattern="[0-9]*" min="0" max="255" value="${data.b}" 
                   onchange="updateValue(${i}, 'b', this.value)">
            <input type="number" class="value-input" inputmode="numeric" pattern="[0-9]*" min="0" max="255" value="${data.v1}" 
                   onchange="updateValue(${i}, 'v1', this.value)">
            <input type="number" class="value-input" inputmode="numeric" pattern="[0-9]*" min="0" max="255" value="${data.v2}" 
                   onchange="updateValue(${i}, 'v2', this.value)">
            <input type="number" class="value-input" inputmode="numeric" pattern="[0-9]*" min="0" max="255" value="${data.v3}" 
                   onchange="updateValue(${i}, 'v3', this.value)">
        `;

        grid.appendChild(row);
    }
}

function updateValue(index, field, value) {
    const numValue = Math.max(0, Math.min(255, parseInt(value) || 0));
    schemaData[index][field] = numValue;

    const row = document.querySelector(`.time-row[data-index="${index}"]`);
    if (row) {
        const preview = row.querySelector('.color-preview');
        const data = schemaData[index];
        preview.style.background = `rgb(${data.r}, ${data.g}, ${data.b})`;
    }
}

function openColorModal(index) {
    currentEditRow = index;
    const data = schemaData[index];

    document.getElementById('rangeR').value = data.r;
    document.getElementById('rangeG').value = data.g;
    document.getElementById('rangeB').value = data.b;

    const hour = Math.floor(index / 2);
    const minute = (index % 2) * 30;
    document.getElementById('modal-time').textContent =
        `${hour.toString().padStart(2, '0')}:${minute.toString().padStart(2, '0')}`;

    updateModalColor();
    document.getElementById('color-modal').classList.add('active');
    document.body.style.overflow = 'hidden';
}

function closeColorModal() {
    document.getElementById('color-modal').classList.remove('active');
    document.body.style.overflow = '';
    currentEditRow = null;
}

function updateModalColor() {
    const r = document.getElementById('rangeR').value;
    const g = document.getElementById('rangeG').value;
    const b = document.getElementById('rangeB').value;

    document.getElementById('valR').textContent = r;
    document.getElementById('valG').textContent = g;
    document.getElementById('valB').textContent = b;

    document.getElementById('preview-large').style.background = `rgb(${r}, ${g}, ${b})`;
}

function applyColor() {
    if (currentEditRow === null) return;

    const r = parseInt(document.getElementById('rangeR').value);
    const g = parseInt(document.getElementById('rangeG').value);
    const b = parseInt(document.getElementById('rangeB').value);

    schemaData[currentEditRow].r = r;
    schemaData[currentEditRow].g = g;
    schemaData[currentEditRow].b = b;

    const row = document.querySelector(`.time-row[data-index="${currentEditRow}"]`);
    if (row) {
        const inputs = row.querySelectorAll('.value-input');
        inputs[0].value = r;
        inputs[1].value = g;
        inputs[2].value = b;
        row.querySelector('.color-preview').style.background = `rgb(${r}, ${g}, ${b})`;
    }

    closeColorModal();
}

async function saveSchema() {
    const schemaFile = document.getElementById('schema-select').value;

    const csv = schemaData.map(row =>
        `${row.r},${row.g},${row.b},${row.v1},${row.v2},${row.v3}`
    ).join('\n');

    try {
        const response = await fetch(`/api/schema/${schemaFile}`, {
            method: 'POST',
            headers: { 'Content-Type': 'text/csv' },
            body: csv
        });

        if (response.ok) {
            showStatus('schema-status', t('schema.saved', { file: schemaFile }), 'success');
        } else {
            throw new Error(t('error'));
        }
    } catch (error) {
        showStatus('schema-status', t('error') + ': ' + error.message, 'error');
    }
}
