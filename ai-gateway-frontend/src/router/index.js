import { createRouter, createWebHistory } from 'vue-router'
import { ElMessage } from 'element-plus'
import { useAuthStore } from '@/stores/auth'

const routes = [
  { path: '/login', name: 'Login', component: () => import('@/views/Login.vue'), meta: { public: true } },
  { path: '/register', name: 'Register', component: () => import('@/views/Register.vue'), meta: { public: true } },
  {
    path: '/',
    component: () => import('@/views/Layout.vue'),
    children: [
      { path: '', redirect: '/dashboard' },
      { path: 'dashboard', name: 'Dashboard', component: () => import('@/views/Dashboard.vue') },
      { path: 'chat', name: 'Chat', component: () => import('@/views/ChatConsole.vue') },
      { path: 'users', name: 'UserManage', component: () => import('@/views/UserManage.vue'), meta: { admin: true } },
      { path: 'models', name: 'ModelConfig', component: () => import('@/views/ModelConfig.vue'), meta: { admin: true } },
      { path: 'packages', name: 'PackageManage', component: () => import('@/views/PackageManage.vue'), meta: { admin: true } },
      { path: 'usage', name: 'UsageStats', component: () => import('@/views/UsageStats.vue') },
      { path: 'devices', name: 'DeviceManage', component: () => import('@/views/DeviceManage.vue') },
      { path: 'audit', name: 'AuditLog', component: () => import('@/views/AuditLog.vue'), meta: { admin: true } },
      { path: 'config', name: 'SystemConfig', component: () => import('@/views/SystemConfig.vue'), meta: { admin: true } },
      { path: 'profile', name: 'Profile', component: () => import('@/views/Profile.vue') },
    ]
  }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

router.beforeEach((to, from, next) => {
  const auth = useAuthStore()
  const isPublic = to.meta.public

  if (!isPublic && !auth.token) {
    next('/login')
    return
  }

  if (to.meta.admin && !auth.user?.is_admin) {
    ElMessage.error('需要管理员权限')
    next('/dashboard')
    return
  }

  next()
})

export default router