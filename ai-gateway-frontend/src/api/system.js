import api from './index'

/**
 * 获取套餐列表
 */
export function getPackages() {
  return api.get('/packages')
}

/**
 * 获取单个套餐
 * @param {string} packageId - 套餐ID
 */
export function getPackage(packageId) {
  return api.get(`/packages/${packageId}`)
}

/**
 * 创建套餐
 * @param {Object} packageData - 套餐数据
 */
export function createPackage(packageData) {
  return api.post('/packages', packageData)
}

/**
 * 更新套餐
 * @param {string} packageId - 套餐ID
 * @param {Object} packageData - 套餐数据
 */
export function updatePackage(packageId, packageData) {
  return api.put(`/packages/${packageId}`, packageData)
}

/**
 * 删除套餐
 * @param {string} packageId - 套餐ID
 */
export function deletePackage(packageId) {
  return api.delete(`/packages/${packageId}`)
}

/**
 * 获取用量统计
 * @param {Object} params - 查询参数
 */
export function getUsageStats(params = {}) {
  return api.get('/stats/usage', { params })
}

/**
 * 获取每日用量统计
 * @param {string} startDate - 开始日期
 * @param {string} endDate - 结束日期
 */
export function getDailyUsageStats(startDate, endDate) {
  return api.get('/stats/daily', { params: { startDate, endDate } })
}

/**
 * 获取模型分布统计
 */
export function getModelDistribution() {
  return api.get('/stats/model-distribution')
}

/**
 * 获取用户用量排行
 * @param {number} limit - 返回数量
 */
export function getTopUsers(limit = 10) {
  return api.get('/stats/top-users', { params: { limit } })
}