import api from './index'
export const scanDevices = () => api.post('/device/scan')
export const getDevices = () => api.get('/device')
export const bindDevice = (id, data) => api.post(`/device/${id}/bind`, data)
export const unbindDevice = (id) => api.post(`/device/${id}/unbind`)
export const sendCommand = (id, data) => api.post(`/device/${id}/command`, data)
export const updateDevice = (id, data) => api.put(`/device/${id}`, data)