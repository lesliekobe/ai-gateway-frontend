<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { useAuthStore } from '@/stores/auth'

const router = useRouter()
const auth = useAuthStore()

const form = ref({ username: '', password: '' })
const loading = ref(false)

async function handleLogin() {
  if (!form.value.username || !form.value.password) return
  loading.value = true
  const ok = await auth.login(form.value.username, form.value.password)
  loading.value = false
  if (ok) router.push('/dashboard')
}
</script>

<template>
  <div class="login-page">
    <el-card class="login-card">
      <h2>AI 网关系统 - 登录</h2>
      <el-form @submit.prevent="handleLogin">
        <el-form-item>
          <el-input v-model="form.username" placeholder="用户名" prefix-icon="User" clearable />
        </el-form-item>
        <el-form-item>
          <el-input v-model="form.password" type="password" placeholder="密码" prefix-icon="Lock" show-password />
        </el-form-item>
        <el-button type="primary" :loading="loading" native-type="submit" style="width:100%">
          登录
        </el-button>
      </el-form>
      <div class="footer">
        <router-link to="/register">没有账号？去注册</router-link>
      </div>
    </el-card>
  </div>
</template>

<style scoped>
.login-page { display: flex; align-items: center; justify-content: center; min-height: 100vh; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); }
.login-card { width: 380px; }
.login-card h2 { text-align: center; margin-bottom: 24px; }
.footer { text-align: center; margin-top: 16px; }
</style>