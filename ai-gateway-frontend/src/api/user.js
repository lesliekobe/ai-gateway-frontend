import api from './index'
export const getUserProfile = () => api.get('/user/profile')
export const updateProfile = (data) => api.put('/user/profile', data)
export const changePassword = (data) => api.post('/user/change-password', data)