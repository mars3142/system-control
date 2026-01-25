<script lang="ts">
    import Toggle from "./Toggle.svelte";
    import {toggleTheme} from "../theme";
    import {onMount} from "svelte";
    import {writable} from "svelte/store";
    import {lang, t} from "../i18n/store";

    const theme = writable<"dark" | "light">("dark");

    function applyInitialTheme() {
        const userTheme = localStorage.getItem("theme");
        if (userTheme) {
            document.documentElement.setAttribute("data-theme", userTheme);
        } else if (window.matchMedia("(prefers-color-scheme: light)").matches) {
            document.documentElement.setAttribute("data-theme", "light");
        } else {
            document.documentElement.setAttribute("data-theme", "dark");
        }
    }

    function updateThemeFromDom() {
        const t = document.documentElement.getAttribute("data-theme");
        theme.set(t === "light" ? "light" : "dark");
    }

    function handleThemeToggle() {
        toggleTheme();
        updateThemeFromDom();
    }

    let themeIcon = $state("🌙");
    let themeLabel = $state("Dark");
    let currentLangCode = $state($lang);
    let currentLang = $state("Deutsch");
    let currentFlag = $state("🇩🇪");

    $effect(() => {
        theme.subscribe(($theme) => {
            themeIcon = $theme === "light" ? "☀️" : "🌙";
            themeLabel = $theme === "light" ? "Light" : "Dark";
        });
        lang.subscribe(($lang) => {
            currentLangCode = $lang;
            currentLang = $lang === "de" ? "Deutsch" : "English";
            currentFlag = $lang === "de" ? "🇩🇪" : "🇬🇧";
        });
    });

    function handleLangChange(newLang: "de" | "en") {
        lang.set(newLang);

        localStorage.setItem("lang", newLang);
    }

    onMount(() => {
        applyInitialTheme();
        updateThemeFromDom();
        window.addEventListener("storage", updateThemeFromDom);

        // Listener für OS-Theme-Änderung
        const mql = window.matchMedia("(prefers-color-scheme: light)");
        const osThemeListener = () => {
            // Nur reagieren, wenn kein User-Theme gesetzt ist
            if (!localStorage.getItem("theme")) {
                applyInitialTheme();
                updateThemeFromDom();
            }
        };
        mql.addEventListener("change", osThemeListener);

        return () => {
            window.removeEventListener("storage", updateThemeFromDom);
            mql.removeEventListener("change", osThemeListener);
        };
    });
</script>

<div class="header">
    <div class="header-controls">
        <Toggle
                label={currentLang}
                icon={currentFlag}
                ariaLabel="Sprache wechseln"
                onClick={() => {
                const newLang = currentLangCode === "de" ? "en" : "de";
                handleLangChange(newLang);
            }}
        />

        <Toggle
                label={themeLabel}
                icon={themeIcon}
                ariaLabel="Theme wechseln"
                onClick={handleThemeToggle}
        />
    </div>
    <h1>🚂 System Control</h1>
</div>

<style>
    .header {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-bottom: 20px;
        flex-wrap: wrap;
        gap: 10px;
    }

    .header-controls {
        display: flex;
        align-items: center;
        gap: 8px;
    }

    @media (max-width: 600px) {
        .header {
            flex-direction: column;
            align-items: stretch;
            text-align: center;
        }

        .header h1 {
            order: 1;
        }
    }

    @media (max-width: 380px) {
        .header h1 {
            font-size: 1.2rem;
        }
    }
</style>
