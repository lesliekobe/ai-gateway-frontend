import { defineStore } from 'pinia'
import api from '@/api/index'

export const useUsersStore = defineStore('users', {
  state: () => ({
    users: [],
    total: 0,
    page: 1,
    pageSize: 20
  }),

  actions: {
    async fetchUsers(params = {}) {
      const res = await api.get('/admin/users', { params: { page: this.page, page_size: this.pageSize, ...params } })
      this.users = res.items || res
      this.total = res.total || this.users.length
    },

    async createUser(data) {
      await api.post('/admin/users', data)
    },

    async updateUser(id, data) {
      await api.put(`/admin/users/${id}`, data)
    },

    async deleteUser(id) {
      await api.delete(`/admin/users/${id}`)
    },

    async resetPassword(id) {
      await api.post(`/admin/users/${id}/reset-password`)
    }
  }
})