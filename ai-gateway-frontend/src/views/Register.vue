<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { useAuthStore } from '@/stores/auth'

const router = useRouter()
const auth = useAuthStore()

const form = ref({ username: '', password: '', email: '' })
const loading = ref(false)

async function handleRegister() {
  if (!form.value.username || !form.value.password || !form.value.email) return
  loading.value = true
  const ok = await auth.register(form.value.username, form.value.password, form.value.email)
  loading.value = false
  if (ok) router.push('/login')
}
</script>

<template>
  <div class="register-page">
    <el-card class="register-card">
      <h2>AI 网关系统 - 注册</h2>
      <el-form @submit.prevent="handleRegister">
        <el-form-item>
          <el-input v-model="form.username" placeholder="用户名" prefix-icon="User" clearable />
        </el-form-item>
        <el-form-item>
          <el-input v-model="form.email" placeholder="邮箱" prefix-icon="Message" clearable />
        </el-form-item>
        <el-form-item>
          <el-input v-model="form.password" type="password" placeholder="密码" prefix-icon="Lock" show-password />
        </el-form-item>
        <el-button type="primary" :loading="loading" native-type="submit" style="width:100%">
          注册
        </el-button>
      </el-form>
      <div class="footer">
        <router-link to="/login">已有账号？去登录</router-link>
      </div>
    </el-card>
  </div>
</template>

<style scoped>
.register-page { display: flex; align-items: center; justify-content: center; min-height: 100vh; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); }
.register-card { width: 380px; }
.register-card h2 { text-align: center; margin-bottom: 24px; }
.footer { text-align: center; margin-top: 16px; }
</style>