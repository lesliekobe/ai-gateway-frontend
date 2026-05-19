<script setup>
import { ref, onMounted } from 'vue'
import { useSystemStore } from '@/stores/system'
import { ElMessage } from 'element-plus'

const system = useSystemStore()
const form = ref({})
const loading = ref(false)
const saving = ref(false)

onMounted(async () => {
  loading.value = true
  try {
    const cfg = await system.fetchConfig()
    form.value = { ...cfg }
  } finally {
    loading.value = false
  }
})

async function handleSave() {
  saving.value = true
  try {
    await system.saveConfig(form.value)
    ElMessage.success('配置已保存')
  } catch (e) {
    ElMessage.error('保存失败')
  } finally {
    saving.value = false
  }
}

async function handleReload() {
  window.location.reload()
}
</script>

<template>
  <div class="system-config">
    <h2>系统配置</h2>
    <el-card v-loading="loading">
      <el-form label-width="180px" size="default">
        <el-divider>服务配置</el-divider>
        <el-form-item label="服务端口">
          <el-input-number v-model="form.server?.port" :min="1024" :max="65535" />
        </el-form-item>
        <el-form-item label="JWT 密钥">
          <el-input v-model="form.jwt?.secret" type="password" show-password />
        </el-form-item>
        <el-form-item label="Access Token 有效期(分钟)">
          <el-input-number v-model="form.jwt?.access_token_expire_minutes" :min="5" />
        </el-form-item>

        <el-divider>AI 模型配置</el-divider>
        <el-form-item label="默认模型">
          <el-select v-model="form.ai?.default_model" style="width:200px">
            <el-option label="DeepSeek" value="deepseek" />
            <el-option label="MiniMax" value="minimax" />
            <el-option label="OpenAI" value="openai" />
          </el-select>
        </el-form-item>
        <el-form-item label="超时时间(秒)">
          <el-input-number v-model="form.ai?.timeout_seconds" :min="10" />
        </el-form-item>
        <el-form-item label="最大重试次数">
          <el-input-number v-model="form.ai?.max_retries" :min="0" />
        </el-form-item>

        <el-divider>限流配置</el-divider>
        <el-form-item label="默认小时限制">
          <el-input-number v-model="form.rate_limit?.default_hourly" :min="1" />
        </el-form-item>
        <el-form-item label="默认日限制">
          <el-input-number v-model="form.rate_limit?.default_daily" :min="1" />
        </el-form-item>
        <el-form-item label="默认并发数">
          <el-input-number v-model="form.rate_limit?.default_concurrent" :min="1" />
        </el-form-item>

        <el-divider>设备扫描配置</el-divider>
        <el-form-item label="启用设备扫描">
          <el-switch v-model="form.device_scan?.enabled" />
        </el-form-item>
        <el-form-item label="扫描间隔(秒)">
          <el-input-number v-model="form.device_scan?.interval_seconds" :min="30" />
        </el-form-item>
        <el-form-item label="子网">
          <el-input v-model="form.device_scan?.subnet" placeholder="192.168.1.0/24" />
        </el-form-item>

        <el-divider>审计配置</el-divider>
        <el-form-item label="日志保留天数">
          <el-input-number v-model="form.audit?.retention_days" :min="7" />
        </el-form-item>
        <el-form-item label="插件目录">
          <el-input v-model="form.plugins?.dir" />
        </el-form-item>
      </el-form>

      <div style="margin-top:24px;display:flex;gap:12px">
        <el-button type="primary" :loading="saving" @click="handleSave">保存配置</el-button>
        <el-button @click="handleReload">重载配置</el-button>
      </div>
    </el-card>
  </div>
</template>