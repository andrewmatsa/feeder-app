import { create } from 'zustand'

interface UiState {
  pageSubtitle: string
  setPageSubtitle: (subtitle: string) => void
  lang: 'uk' | 'en'
  setLang: (lang: 'uk' | 'en') => void
}

export const useUiStore = create<UiState>((set) => ({
  pageSubtitle: '',
  setPageSubtitle: (subtitle) => set({ pageSubtitle: subtitle }),
  lang: ((localStorage.getItem('aq_lang') ?? localStorage.getItem('lang')) as 'uk' | 'en') ?? 'uk',
  setLang: (lang) => { localStorage.setItem('lang', lang); set({ lang }) },
}))
