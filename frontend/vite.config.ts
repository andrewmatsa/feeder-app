import { resolve } from 'node:path'
import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

const backendUrl = process.env.BACKEND_URL || 'http://localhost:8000'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [react()],
  server: {
    host: true,
    port: 5173,
    fs: {
      allow: [resolve(__dirname, '..')],
    },
    proxy: {
      '/api': { target: backendUrl, changeOrigin: true },
      '/auth': { target: backendUrl, changeOrigin: true },
    },
    watch: {
      usePolling: true,
    },
  },
  build: {
    outDir: 'dist',
    sourcemap: true,
  },
})
