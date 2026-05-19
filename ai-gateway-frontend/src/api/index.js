import axios from 'axios'
import { ElMessage } from 'element-plus'
import router from '@/router'

// 创建axios实例
const api = axios.create({
  baseURL: '/api',
  timeout: 30000,
  headers: {
    'Content-Type': 'application/json'
  }
})

// 请求拦截器 - 添加Token
api.interceptors.request.use(
  (config) => {
    const token = localStorage.getItem('token')
    if (token) {
      config.headers.Authorization = `Bearer ${token}`
    }
    return config
  },
  (error) => {
    return Promise.reject(error)
  }
)

// 响应拦截器 - 处理错误和Token刷新
api.interceptors.response.use(
  (response) => {
    return response.data
  },
  async (error) => {
    if (error.response) {
      const { status, data } = error.response

      switch (status) {
        case 401:
          // Token过期，尝试刷新
          const refreshToken = localStorage.getItem('refreshToken')
          if (refreshToken) {
            try {
              const res = await axios.post('/api/auth/refresh', { refreshToken })
              localStorage.setItem('token', res.data.token)
              // 重试原请求
              error.config.headers.Authorization = `Bearer ${res.data.token}`
              return api.request(error.config)
            } catch (refreshError) {
              // 刷新失败，清除token并跳转登录
              localStorage.removeItem('token')
              localStorage.removeItem('refreshToken')
              router.push('/login')
              ElMessage.error('登录已过期，请重新登录')
            }
          } else {
            router.push('/login')
            ElMessage.error('请先登录')
          }
          break

        case 403:
          ElMessage.error('没有权限访问')
          break

        case 404:
          ElMessage.error('请求的资源不存在')
          break

        case 500:
          ElMessage.error('服务器内部错误')
          break

        case 429:
          ElMessage.warning('请求过于频繁，请稍后再试')
          break

        default:
          ElMessage.error(data.message || '请求失败')
      }
    } else if (error.request) {
      ElMessage.error('网络连接失败，请检查网络')
    } else {
      ElMessage.error('请求配置错误')
    }

    return Promise.reject(error)
  }
)

export default api