<script setup>
import { useRouter } from 'vue-router'
import { useAuthStore } from '@/stores/auth'
import { ElMessageBox } from 'element-plus'

const router = useRouter()
const auth = useAuthStore()
const emit = defineEmits(['toggle'])

async function handleLogout() {
  await ElMessageBox.confirm('确定退出登录？', '提示')
  await auth.logout()
  router.push('/login')
}
</script>

<template>
  <div class="header">
    <el-button text @click="$emit('toggle')">
      <el-icon :size="20"><Fold /></el-icon>
    </el-button>
    <div class="right">
      <span style="margin-right:12px">欢迎, {{ auth.user?.username || '用户' }}</span>
      <el-button type="danger" size="small" @click="handleLogout">退出</el-button>
    </div>
  </div>
</template>

<style scoped>
.header { display: flex; justify-content: space-between; align-items: center; width: 100%; }
.right { display: flex; align-items: center; }
</style>