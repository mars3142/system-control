---
description: "Validate frontend changes: build, test, and check for regressions against API expectations and existing features."
name: "Website Change Check"
argument-hint: "Summary of changes made (e.g., routing update, new component, dependency upgrade)"
agent: "agent"
---
# Website Change Check

Run a quick validation pass on frontend changes:

**Steps:**
1. **Build Check**: Run `npm run build` and report any TypeScript/Svelte errors
2. **Test Check**: Run `npm run test` and report test coverage or failures
3. **API Compatibility**: Verify endpoint usage (search codebase for /api calls) matches current routes in [README-API.md](README-API.md)
4. **Asset Integrity**: Confirm static asset paths and fetch patterns still work with firmware's static-file server (including gzip fallback)
5. **Summary**: List any breaking changes or build artifacts that need cleanup

**Context:**
- Website builds and tests must pass for firmware flashing
- Static assets are served by [api-server](components/api-server/src/api_handlers.c#L898) with potential gzip variants
- Captive portal access (AP mode) expects `/`, `/index.html` → redirect to `/captive.html`

**Reference:**
- [website/package.json](website/package.json)—build and test scripts
- [.github/instructions/website.instructions.md](.github/instructions/website.instructions.md)—frontend conventions

**Expected Output:**
- ✅ or ❌ for each check
- Specific error messages or warnings
- Suggested fixes (if any)
