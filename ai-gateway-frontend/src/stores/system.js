import { defineStore } from 'pinia'
import api from '@/api/index'

export const useSystemStore = defineStore('system', {
  state: () => ({
    stats: {},
    config: {},
    packages: [],
    usageData: []
  }),

  actions: {
    async fetchDashboard() {
      const res = await api.get('/admin/dashboard')
      this.stats = res
      return res
    },

    async fetchConfig() {
      const res = await api.get('/config')
      this.config = res
      return res
    },

    async saveConfig(cfg) {
      await api.put('/config', cfg)
    },

    async fetchPackages() {
      const res = await api.get('/package')
      this.packages = res
      return res
    },

    async fetchUsage(params) {
      const res = await api.get('/admin/usage', { params })
      this.usageData = res
      return res
    }
  }
})