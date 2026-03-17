---
description: "Use when editing Svelte/Vite frontend files, routing, styling, static assets, and frontend build or test scripts under website."
name: "Website Instructions"
applyTo: "website/**"
---
# Website Guidelines

- Keep changes aligned with the existing Svelte + Vite setup and current dependency stack in website/package.json.
- Validate frontend changes with project scripts:
  - npm run build
  - npm run test
- When changing static assets, keep compatibility with firmware static-file serving expectations:
  - prefer deterministic asset names/paths used by the frontend bundle
  - ensure assets are safe to serve with optional gzip variants
- Avoid introducing framework or tooling migrations unless explicitly requested.
- Keep UI behavior robust for captive-portal style access patterns (direct root load, limited network conditions).
- For API usage from frontend, preserve existing endpoint conventions under /api and avoid ad-hoc route shape changes.
