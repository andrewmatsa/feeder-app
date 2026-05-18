/// <reference types="vite/client" />

interface ImportMetaEnv {
  /** Leave unset in Docker dev — use Vite proxy to backend */
  readonly VITE_API_URL?: string
}

interface ImportMeta {
  readonly env: ImportMetaEnv
}
