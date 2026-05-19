<script setup>
import { computed } from 'vue'

const props = defineProps({
  collapsed: { type: Boolean, default: false }
})

const menuItems = [
  { path: '/dashboard', icon: 'DataLine', label: '数据大盘' },
  { path: '/chat', icon: 'ChatDotRound', label: 'AI 对话' },
  { path: '/users', icon: 'User', label: '用户管理', admin: true },
  { path: '/models', icon: 'Setting', label: '模型配置', admin: true },
  { path: '/packages', icon: 'Goods', label: '套餐管理', admin: true },
  { path: '/usage', icon: 'TrendCharts', label: '用量统计' },
  { path: '/devices', icon: 'Monitor', label: '设备管理' },
  { path: '/audit', icon: 'Document', label: '审计日志', admin: true },
  { path: '/config', icon: 'Tools', label: '系统配置', admin: true },
  { path: '/profile', icon: 'UserFilled', label: '个人设置' },
]

import * as ElementPlusIconsVue from '@element-plus/icons-vue'
</script>

<template>
  <div class="sidebar" :class="{ collapsed }">
    <div class="logo" v-if="!collapsed">AI Gateway</div>
    <div class="logo" v-else>AG</div>
    <el-menu :collapse="collapsed" :default-active="$route.path" router>
      <el-menu-item v-for="item in menuItems" :key="item.path" :index="item.path">
        <el-icon><component :is="item.icon" /></el-icon>
        <template #title>{{ item.label }}</template>
      </el-menu-item>
    </el-menu>
  </div>
</template>

<style scoped>
.sidebar { background: #001529; height: 100vh; transition: width 0.3s; }
.logo { height: 60px; line-height: 60px; text-align: center; color: #fff; font-size: 18px; font-weight: bold; border-bottom: 1px solid #ffffff20; }
:deep(.el-menu) { background: transparent; border: none; }
:deep(.el-menu-item) { color: #fff; }
:deep(.el-menu-item:hover) { background: #ffffff10; }
:deep(.el-menu-item.is-active) { background: #409EFF30; color: #409EFF; }
</style>