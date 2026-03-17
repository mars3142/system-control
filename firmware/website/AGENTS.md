# AGENTS.md

## Project Overview
This is a Svelte-based web frontend for a model railway system control, using Vite for build tooling and SPA routing via `svelte-spa-router`. The app is structured around two main tabs: Control and Config, with a captive portal route for special cases.

## Architecture & Data Flow
- **Entry Point:** `src/main.js` mounts the root Svelte component (`app.svelte`) to the DOM.
- **Routing:** Defined in `app.svelte` using `svelte-spa-router`. Routes are:
  - `/` → redirects to `/control`
  - `/control` → ControlTab
  - `/config` → ConfigTab
  - `/captive` → Captive Portal
- **State Management:**
  - Centralized via Svelte stores, especially `controlStore` in `src/stores/controlStore.ts`.
  - `controlStore` manages system state, communicates with backend via REST and WebSocket, and exposes methods for light, mode, and schema changes.
  - WebSocket auto-reconnects and is only active when there are subscribers.
- **Internationalization:**
  - Language state managed in `src/i18n/store.ts`.
  - Translations loaded from `src/i18n/de.json` and `src/i18n/en.json`.
  - Language switching persists in `localStorage`.

## Developer Workflows
- **Build:** `npm run build` (outputs to `../storage/website`)
- **Dev Server:** `npm run dev` (port 5173, strict)
- **Preview:** `npm run preview`
- **Testing:** `npm run test` (uses Vitest, tests in `src/stores/controlStore.test.ts`)
- **Versioning:**
  - App version and commit hash injected via `vite.config.js` (from `../version.txt` and `git rev-parse`).
  - Displayed in footer.

## Project-Specific Patterns
- **Tab Navigation:**
  - Implemented via `TabBar` and `TabButton` components.
  - Active tab is derived from route.
- **Control Logic:**
  - All control actions (light, mode, schema) use latest-only sender pattern to avoid race conditions.
  - REST endpoints: `/api/light/status`, `/api/light/power`, `/api/light/mode`, `/api/light/schema`.
  - WebSocket endpoint: `/ws`.
- **Styling:**
  - TailwindCSS and PicoCSS used for styling.
  - Custom themes and responsive layouts.
- **Component Structure:**
  - Common UI elements in `src/components/common/`.
  - Control-specific UI in `src/components/control/`.
  - Config-specific UI in `src/components/config/`.

## Integration Points
- **Backend:**
  - Communicates via REST and WebSocket (see endpoints above).
  - Host resolution adapts for dev/prod.
- **External Libraries:**
  - GSAP for animations.
  - TailwindCSS, PicoCSS, Fontsource for UI.

## Examples
- To add a new control action, extend `controlStore` and update relevant Svelte components.
- To add a new language, add a JSON file in `src/i18n/` and update `translations` in `index.ts`.
- To debug state, use browser devtools and subscribe to Svelte stores.

## Key Files
- `src/app.svelte` (routing, layout)
- `src/stores/controlStore.ts` (state, backend communication)
- `src/i18n/store.ts` (language state)
- `vite.config.js` (build, versioning)
- `src/components/` (UI)

_Last updated: 2026-03-17_
