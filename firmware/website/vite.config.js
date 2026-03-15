/// <reference types="vitest/config" />
import { defineConfig } from 'vite';
import { svelte } from '@sveltejs/vite-plugin-svelte';
import viteCompression from 'vite-plugin-compression';
import { execSync } from 'child_process';
import fs from 'fs';
import path from 'path';

const commitHash = execSync('git rev-parse --short HEAD').toString().trim();
const version = fs.readFileSync(path.resolve(__dirname, '../version.txt'), 'utf-8').trim();

export default defineConfig({
  define: {
    __COMMIT_HASH__: JSON.stringify(commitHash),
    __APP_VERSION__: JSON.stringify(version),
  },
  plugins: [
    svelte({
      configFile: 'svelte.config.js'
    }),
    viteCompression({
      algorithm: 'gzip',
      ext: '.gz',
      deleteOriginFile: true,
    }),
  ],
  build: {
    outDir: '../storage/website',
    assetsDir: '',
    sourcemap: true,
    rollupOptions: {
      output: {
        manualChunks(id) {
          if (id.includes('node_modules')) {
            return 'vendor';
          }
        },
      },
    },
  },
  server: {
    port: 5173,
    strictPort: true,
    sourcemapIgnoreList(sourcePath) {
      return sourcePath.includes('node_modules');
    }
  }
});