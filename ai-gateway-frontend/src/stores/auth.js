import { defineStore } from 'pinia'
import api from '@/api/index'
import { ElMessage } from 'element-plus'

export const useAuthStore = defineStore('auth', {
  state: () => ({
    token: localStorage.getItem('token') || '',
    refreshToken: localStorage.getItem('refreshToken') || '',
    user: null
  }),

  actions: {
    async login(username, password) {
      try {
        const res = await api.post('/auth/login', { username, password })
        this.token = res.access_token
        this.refreshToken = res.refresh_token
        localStorage.setItem('token', this.token)
        localStorage.setItem('refreshToken', this.refreshToken)
        await this.fetchProfile()
        return true
      } catch (e) {
        ElMessage.error(e.response?.data?.detail || '登录失败')
        return false
      }
    },

    async register(username, password, email) {
      try {
        await api.post('/auth/register', { username, password, email })
        ElMessage.success('注册成功，请登录')
        return true
      } catch (e) {
        ElMessage.error(e.response?.data?.detail || '注册失败')
        return false
      }
    },

    async fetchProfile() {
      try {
        const res = await api.get('/user/profile')
        this.user = res
      } catch (e) {
        console.error('fetchProfile failed', e)
      }
    },

    async logout() {
      try {
        await api.post('/auth/logout')
      } catch (e) { /* ignore */ }
      this.token = ''
      this.refreshToken = ''
      this.user = null
      localStorage.removeItem('token')
      localStorage.removeItem('refreshToken')
    },

    clearAuth() {
      this.token = ''
      this.refreshToken = ''
      this.user = null
      localStorage.removeItem('token')
      localStorage.removeItem('refreshToken')
    }
  }
})