<script setup>
import { ref } from 'vue'
import { useAuthStore } from '@/stores/auth'
import { ElMessage } from 'element-plus'
import api from '@/api/user'

const auth = useAuthStore()
const passwordForm = ref({ old: '', new1: '', new2: '' })
const saving = ref(false)

async function handleChangePassword() {
  if (passwordForm.value.new1 !== passwordForm.value.new2) {
    ElMessage.error('两次新密码不一致')
    return
  }
  saving.value = true
  try {
    await api.changePassword({ old_password: passwordForm.value.old, new_password: passwordForm.value.new1 })
    ElMessage.success('密码修改成功')
    passwordForm.value = { old: '', new1: '', new2: '' }
  } catch (e) {
    ElMessage.error('修改失败')
  } finally {
    saving.value = false
  }
}
</script>

<template>
  <div class="profile">
    <h2>个人设置</h2>
    <el-row :gutter="16">
      <el-col :span="12">
        <el-card title="修改密码">
          <el-form label-width="120px">
            <el-form-item label="旧密码">
              <el-input v-model="passwordForm.old" type="password" show-password />
            </el-form-item>
            <el-form-item label="新密码">
              <el-input v-model="passwordForm.new1" type="password" show-password />
            </el-form-item>
            <el-form-item label="确认新密码">
              <el-input v-model="passwordForm.new2" type="password" show-password />
            </el-form-item>
            <el-form-item>
              <el-button type="primary" :loading="saving" @click="handleChangePassword">修改密码</el-button>
            </el-form-item>
          </el-form>
        </el-card>
      </el-col>
      <el-col :span="12">
        <el-card title="账户信息">
          <el-descriptions :column="1" border>
            <el-descriptions-item label="用户名">{{ auth.user?.username }}</el-descriptions-item>
            <el-descriptions-item label="邮箱">{{ auth.user?.email }}</el-descriptions-item>
            <el-descriptions-item label="管理员">
              <el-tag :type="auth.user?.is_admin ? 'danger' : 'success'">
                {{ auth.user?.is_admin ? '是' : '否' }}
              </el-tag>
            </el-descriptions-item>
            <el-descriptions-item label="会员套餐">{{ auth.user?.package_name || '未开通' }}</el-descriptions-item>
          </el-descriptions>
        </el-card>
      </el-col>
    </el-row>
  </div>
</template>