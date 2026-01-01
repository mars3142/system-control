// Internationalization (i18n) - Language support
// Supported languages: German (de), English (en)

const translations = {
    de: {
        // Page
        'page.title': 'System Control',

        // Main Tabs
        'tab.control': '🎛️ Bedienung',
        'tab.config': '⚙️ Konfiguration',

        // Sub Tabs
        'subtab.wifi': '📶 WLAN',
        'subtab.light': '💡 Lichtsteuerung',
        'subtab.devices': '🔗 Geräte',
        'subtab.scenes': '🎬 Szenen',

        // LED Configuration
        'wled.config.title': 'LED Konfiguration',
        'wled.config.desc': 'Konfiguriere die LED-Segmente und Anzahl LEDs pro Segment',
        'wled.segments.title': 'Segmente',
        'wled.segments.empty': 'Keine Segmente konfiguriert',
        'wled.segments.empty.hint': 'Klicke auf "Segment hinzufügen" um ein Segment zu erstellen',
        'wled.segment.add': '➕ Segment hinzufügen',
        'wled.segment.name': 'Segment {num}',
        'wled.segment.leds': 'Anzahl LEDs',
        'wled.segment.start': 'Start-LED',
        'wled.segment.remove': 'Entfernen',
        'wled.saved': 'LED-Konfiguration gespeichert!',
        'wled.error.save': 'Fehler beim Speichern der LED-Konfiguration',
        'wled.loaded': 'LED-Konfiguration geladen',

        // Light Control
        'control.light.title': 'Lichtsteuerung',
        'control.light.light': 'Licht',
        'control.light.thunder': 'Gewitter',
        'control.mode.title': 'Betriebsmodus',
        'control.schema.active': 'Aktives Schema',
        'control.status.title': 'Aktueller Status',
        'control.status.mode': 'Modus',
        'control.status.schema': 'Schema',
        'control.status.color': 'Aktuelle Farbe',

        // Common
        'common.on': 'AN',
        'common.off': 'AUS',
        'common.loading': 'Wird geladen...',

        // Modes
        'mode.day': 'Tag',
        'mode.night': 'Nacht',
        'mode.simulation': 'Simulation',

        // Schema names
        'schema.name.1': 'Schema 1 (Standard)',
        'schema.name.2': 'Schema 2 (Warm)',
        'schema.name.3': 'Schema 3 (Natur)',

        // Scenes
        'scenes.title': 'Szenen',
        'scenes.empty': 'Keine Szenen definiert',
        'scenes.empty.hint': 'Erstelle Szenen unter Konfiguration',
        'scenes.manage.title': 'Szenen verwalten',
        'scenes.manage.desc': 'Erstelle und bearbeite Szenen für schnellen Zugriff',
        'scenes.config.empty': 'Keine Szenen erstellt',
        'scenes.config.empty.hint': 'Klicke auf "Neue Szene" um eine Szene zu erstellen',
        'scenes.activated': '"{name}" aktiviert',
        'scenes.created': 'Szene erstellt',
        'scenes.updated': 'Szene aktualisiert',
        'scenes.deleted': '"{name}" gelöscht',
        'scenes.confirm.delete': '"{name}" wirklich löschen?',
        'scenes.error.name': 'Bitte Namen eingeben',

        // Devices
        'devices.external': 'Externe Geräte',
        'devices.control.empty': 'Keine Geräte hinzugefügt',
        'devices.control.empty.hint': 'Füge Geräte unter Konfiguration hinzu',
        'devices.new.title': 'Neue Geräte',
        'devices.new.desc': 'Unprovisionierte Matter-Geräte in der Nähe',
        'devices.searching': 'Suche nach Geräten...',
        'devices.unpaired.empty': 'Keine neuen Geräte gefunden',
        'devices.unpaired.empty.hint': 'Drücke "Geräte suchen" um nach Matter-Geräten zu suchen',
        'devices.paired.title': 'Zugeordnete Geräte',
        'devices.paired.desc': 'Bereits hinzugefügte externe Geräte',
        'devices.paired.empty': 'Keine Geräte hinzugefügt',
        'devices.none.available': 'Keine Geräte verfügbar',
        'devices.found': '{count} Gerät(e) gefunden',
        'devices.added': '"{name}" erfolgreich hinzugefügt',
        'devices.removed': '"{name}" entfernt',
        'devices.name.updated': 'Name aktualisiert',
        'devices.confirm.remove': '"{name}" wirklich entfernen?',

        // WiFi
        'wifi.config.title': 'WLAN Konfiguration',
        'wifi.ssid': 'WLAN Name (SSID)',
        'wifi.ssid.placeholder': 'Netzwerkname eingeben',
        'wifi.password': 'WLAN Passwort',
        'wifi.password.short': 'Passwort',
        'wifi.password.placeholder': 'Passwort eingeben',
        'wifi.available': 'Verfügbare Netzwerke',
        'wifi.scan.hint': 'Nach Netzwerken suchen...',
        'wifi.status.title': 'Verbindungsstatus',
        'wifi.status.status': 'Status:',
        'wifi.status.ip': 'IP-Adresse:',
        'wifi.status.signal': 'Signal:',
        'wifi.connected': '✅ Verbunden',
        'wifi.disconnected': '❌ Nicht verbunden',
        'wifi.unavailable': '⚠️ Status nicht verfügbar',
        'wifi.searching': 'Suche läuft...',
        'wifi.scan.error': 'Fehler beim Scannen',
        'wifi.scan.failed': 'Netzwerksuche fehlgeschlagen',
        'wifi.saved': 'WLAN-Konfiguration gespeichert! Gerät verbindet sich...',
        'wifi.error.ssid': 'Bitte WLAN-Name eingeben',
        'wifi.error.save': 'Fehler beim Speichern',
        'wifi.networks.found': '{count} Netzwerk(e) gefunden',

        // Schema Editor
        'schema.editor.title': 'Licht-Schema Editor',
        'schema.file': 'Schema-Datei',
        'schema.loading': 'Schema wird geladen...',
        'schema.header.time': 'Zeit',
        'schema.header.color': 'Farbe',
        'schema.loaded': '{file} erfolgreich geladen',
        'schema.saved': '{file} erfolgreich gespeichert!',
        'schema.demo': 'Demo-Daten geladen (Server nicht erreichbar)',

        // Color Modal
        'modal.color.title': 'Farbe wählen',

        // Scene Modal
        'modal.scene.new': 'Neue Szene erstellen',
        'modal.scene.edit': 'Szene bearbeiten',
        'scene.name': 'Name',
        'scene.name.placeholder': 'z.B. Abendstimmung',
        'scene.icon': 'Icon auswählen',
        'scene.actions': 'Aktionen',
        'scene.action.light': 'Licht Ein/Aus',
        'scene.action.mode': 'Modus setzen',
        'scene.action.schema': 'Schema wählen',
        'scene.light.on': 'Einschalten',
        'scene.light.off': 'Ausschalten',

        // Buttons
        'btn.scan': '🔍 Suchen',
        'btn.save': '💾 Speichern',
        'btn.load': '🔄 Laden',
        'btn.cancel': 'Abbrechen',
        'btn.apply': 'Übernehmen',
        'btn.new.scene': '➕ Neue Szene',
        'btn.scan.devices': '🔍 Geräte suchen',
        'btn.add': 'Hinzufügen',
        'btn.remove': 'Entfernen',
        'btn.edit': 'Bearbeiten',
        'btn.delete': 'Löschen',

        // Captive Portal
        'captive.title': 'System Control - WLAN Setup',
        'captive.subtitle': 'WLAN-Einrichtung',
        'captive.scan': '📡 Netzwerke suchen',
        'captive.scanning': 'Suche nach Netzwerken...',
        'captive.or.manual': 'oder manuell eingeben',
        'captive.password.placeholder': 'WLAN-Passwort',
        'captive.connect': '💾 Verbinden',
        'captive.note.title': 'Hinweis:',
        'captive.note.text': 'Nach dem Speichern verbindet sich das Gerät mit dem gewählten Netzwerk. Diese Seite wird dann nicht mehr erreichbar sein. Verbinden Sie sich mit Ihrem normalen WLAN, um auf das Gerät zuzugreifen.',
        'captive.connecting': 'Verbindung wird hergestellt... {seconds}s',
        'captive.done': 'Gerät sollte jetzt verbunden sein. Sie können diese Seite schließen.',

        // General
        'loading': 'Laden...',
        'error': 'Fehler',
        'success': 'Erfolg'
    },

    en: {
        // Page
        'page.title': 'System Control',

        // Main Tabs
        'tab.control': '🎛️ Control',
        'tab.config': '⚙️ Settings',

        // Sub Tabs
        'subtab.wifi': '📶 WiFi',
        'subtab.light': '💡 Light Control',
        'subtab.devices': '🔗 Devices',
        'subtab.scenes': '🎬 Scenes',

        // LED Configuration
        'wled.config.title': 'LED Configuration',
        'wled.config.desc': 'Configure LED segments and number of LEDs per segment',
        'wled.segments.title': 'Segments',
        'wled.segments.empty': 'No segments configured',
        'wled.segments.empty.hint': 'Click "Add Segment" to create a segment',
        'wled.segment.add': '➕ Add Segment',
        'wled.segment.name': 'Segment {num}',
        'wled.segment.leds': 'Number of LEDs',
        'wled.segment.start': 'Start LED',
        'wled.segment.remove': 'Remove',
        'wled.saved': 'LED configuration saved!',
        'wled.error.save': 'Error saving LED configuration',
        'wled.loaded': 'LED configuration loaded',

        // Light Control
        'control.light.title': 'Light Control',
        'control.light.light': 'Light',
        'control.light.thunder': 'Thunder',
        'control.mode.title': 'Operating Mode',
        'control.schema.active': 'Active Schema',
        'control.status.title': 'Current Status',
        'control.status.mode': 'Mode',
        'control.status.schema': 'Schema',
        'control.status.color': 'Current Color',

        // Common
        'common.on': 'ON',
        'common.off': 'OFF',
        'common.loading': 'Loading...',

        // Modes
        'mode.day': 'Day',
        'mode.night': 'Night',
        'mode.simulation': 'Simulation',

        // Schema names
        'schema.name.1': 'Schema 1 (Standard)',
        'schema.name.2': 'Schema 2 (Warm)',
        'schema.name.3': 'Schema 3 (Natural)',

        // Scenes
        'scenes.title': 'Scenes',
        'scenes.empty': 'No scenes defined',
        'scenes.empty.hint': 'Create scenes in settings',
        'scenes.manage.title': 'Manage Scenes',
        'scenes.manage.desc': 'Create and edit scenes for quick access',
        'scenes.config.empty': 'No scenes created',
        'scenes.config.empty.hint': 'Click "New Scene" to create a scene',
        'scenes.activated': '"{name}" activated',
        'scenes.created': 'Scene created',
        'scenes.updated': 'Scene updated',
        'scenes.deleted': '"{name}" deleted',
        'scenes.confirm.delete': 'Really delete "{name}"?',
        'scenes.error.name': 'Please enter a name',

        // Devices
        'devices.external': 'External Devices',
        'devices.control.empty': 'No devices added',
        'devices.control.empty.hint': 'Add devices in settings',
        'devices.new.title': 'New Devices',
        'devices.new.desc': 'Unprovisioned Matter devices nearby',
        'devices.searching': 'Searching for devices...',
        'devices.unpaired.empty': 'No new devices found',
        'devices.unpaired.empty.hint': 'Press "Scan devices" to search for Matter devices',
        'devices.paired.title': 'Paired Devices',
        'devices.paired.desc': 'Already added external devices',
        'devices.paired.empty': 'No devices added',
        'devices.none.available': 'No devices available',
        'devices.found': '{count} device(s) found',
        'devices.added': '"{name}" added successfully',
        'devices.removed': '"{name}" removed',
        'devices.name.updated': 'Name updated',
        'devices.confirm.remove': 'Really remove "{name}"?',

        // WiFi
        'wifi.config.title': 'WiFi Configuration',
        'wifi.ssid': 'WiFi Name (SSID)',
        'wifi.ssid.placeholder': 'Enter network name',
        'wifi.password': 'WiFi Password',
        'wifi.password.short': 'Password',
        'wifi.password.placeholder': 'Enter password',
        'wifi.available': 'Available Networks',
        'wifi.scan.hint': 'Search for networks...',
        'wifi.status.title': 'Connection Status',
        'wifi.status.status': 'Status:',
        'wifi.status.ip': 'IP Address:',
        'wifi.status.signal': 'Signal:',
        'wifi.connected': '✅ Connected',
        'wifi.disconnected': '❌ Not connected',
        'wifi.unavailable': '⚠️ Status unavailable',
        'wifi.searching': 'Searching...',
        'wifi.scan.error': 'Scan error',
        'wifi.scan.failed': 'Network scan failed',
        'wifi.saved': 'WiFi configuration saved! Device connecting...',
        'wifi.error.ssid': 'Please enter WiFi name',
        'wifi.error.save': 'Error saving',
        'wifi.networks.found': '{count} network(s) found',

        // Schema Editor
        'schema.editor.title': 'Light Schema Editor',
        'schema.file': 'Schema File',
        'schema.loading': 'Loading schema...',
        'schema.header.time': 'Time',
        'schema.header.color': 'Color',
        'schema.loaded': '{file} loaded successfully',
        'schema.saved': '{file} saved successfully!',
        'schema.demo': 'Demo data loaded (server unreachable)',

        // Color Modal
        'modal.color.title': 'Choose Color',

        // Scene Modal
        'modal.scene.new': 'Create New Scene',
        'modal.scene.edit': 'Edit Scene',
        'scene.name': 'Name',
        'scene.name.placeholder': 'e.g. Evening Mood',
        'scene.icon': 'Choose Icon',
        'scene.actions': 'Actions',
        'scene.action.light': 'Light On/Off',
        'scene.action.mode': 'Set Mode',
        'scene.action.schema': 'Choose Schema',
        'scene.light.on': 'Turn On',
        'scene.light.off': 'Turn Off',

        // Buttons
        'btn.scan': '🔍 Scan',
        'btn.save': '💾 Save',
        'btn.load': '🔄 Load',
        'btn.cancel': 'Cancel',
        'btn.apply': 'Apply',
        'btn.new.scene': '➕ New Scene',
        'btn.scan.devices': '🔍 Scan Devices',
        'btn.add': 'Add',
        'btn.remove': 'Remove',
        'btn.edit': 'Edit',
        'btn.delete': 'Delete',

        // Captive Portal
        'captive.title': 'System Control - WiFi Setup',
        'captive.subtitle': 'WiFi Setup',
        'captive.scan': '📡 Scan Networks',
        'captive.scanning': 'Scanning for networks...',
        'captive.or.manual': 'or enter manually',
        'captive.password.placeholder': 'WiFi password',
        'captive.connect': '💾 Connect',
        'captive.note.title': 'Note:',
        'captive.note.text': 'After saving, the device will connect to the selected network. This page will no longer be accessible. Connect to your regular WiFi to access the device.',
        'captive.connecting': 'Connecting... {seconds}s',
        'captive.done': 'Device should now be connected. You can close this page.',

        // General
        'loading': 'Loading...',
        'error': 'Error',
        'success': 'Success'
    }
};

// Current language
let currentLang = localStorage.getItem('lang') || 'de';

/**
 * Get translation for a key
 * @param {string} key - Translation key
 * @param {object} params - Optional parameters for interpolation
 * @returns {string} Translated text
 */
function t(key, params = {}) {
    const lang = translations[currentLang] || translations.de;
    let text = lang[key] || translations.de[key] || key;

    // Replace parameters like {count}, {name}, etc.
    Object.keys(params).forEach(param => {
        text = text.replace(new RegExp(`\\{${param}\\}`, 'g'), params[param]);
    });

    return text;
}

/**
 * Set current language
 * @param {string} lang - Language code ('de' or 'en')
 */
function setLanguage(lang) {
    if (translations[lang]) {
        currentLang = lang;
        localStorage.setItem('lang', lang);
        document.documentElement.lang = lang;
        updatePageLanguage();
        updateLanguageToggle();
    }
}

/**
 * Toggle between languages
 */
function toggleLanguage() {
    setLanguage(currentLang === 'de' ? 'en' : 'de');
}

/**
 * Get current language
 * @returns {string} Current language code
 */
function getCurrentLanguage() {
    return currentLang;
}

/**
 * Update all elements with data-i18n attribute
 */
function updatePageLanguage() {
    // Update elements with data-i18n attribute
    document.querySelectorAll('[data-i18n]').forEach(el => {
        const key = el.getAttribute('data-i18n');
        const translated = t(key);
        if (translated !== key) {
            el.textContent = translated;
        }
    });

    // Update elements with data-i18n-placeholder attribute
    document.querySelectorAll('[data-i18n-placeholder]').forEach(el => {
        const key = el.getAttribute('data-i18n-placeholder');
        const translated = t(key);
        if (translated !== key) {
            el.placeholder = translated;
        }
    });

    // Update elements with data-i18n-title attribute
    document.querySelectorAll('[data-i18n-title]').forEach(el => {
        const key = el.getAttribute('data-i18n-title');
        const translated = t(key);
        if (translated !== key) {
            el.title = translated;
        }
    });

    // Update elements with data-i18n-aria attribute
    document.querySelectorAll('[data-i18n-aria]').forEach(el => {
        const key = el.getAttribute('data-i18n-aria');
        const translated = t(key);
        if (translated !== key) {
            el.setAttribute('aria-label', translated);
        }
    });

    // Update page title
    const titleEl = document.querySelector('title[data-i18n]');
    if (titleEl) {
        document.title = t(titleEl.getAttribute('data-i18n'));
    }
}

/**
 * Update language toggle button
 */
function updateLanguageToggle() {
    const langFlag = document.getElementById('lang-flag');
    const langLabel = document.getElementById('lang-label');

    if (langFlag) {
        langFlag.textContent = currentLang === 'de' ? '🇩🇪' : '🇬🇧';
    }
    if (langLabel) {
        langLabel.textContent = currentLang.toUpperCase();
    }
}

/**
 * Initialize i18n
 */
function initI18n() {
    // Check browser language as fallback
    if (!localStorage.getItem('lang')) {
        const browserLang = navigator.language.split('-')[0];
        if (translations[browserLang]) {
            currentLang = browserLang;
        }
    }

    document.documentElement.lang = currentLang;
    updatePageLanguage();
    updateLanguageToggle();
}
