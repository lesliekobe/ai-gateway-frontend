import api from './index'
export const login = (data) => api.post('/auth/login', data)
export const register = (data) => api.post('/auth/register', data)
export const logout = () => api.post('/auth/logout')
export const refresh = (data) => api.post('/auth/refresh', data)