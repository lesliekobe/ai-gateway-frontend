import api from './index'
export const chat = (data) => api.post('/chat', data)
export const chatStream = (data) => api.post('/chat/stream', data, { responseType: 'stream' })
export const getHistory = (params) => api.get('/chat/history', { params })