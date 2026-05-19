import api from './index'
export const getAuditLogs = (params) => api.get('/audit', { params })
export const exportAuditLogs = (params) => api.get('/audit/export', { params, responseType: 'blob' })
export const getActions = () => api.get('/audit/actions')