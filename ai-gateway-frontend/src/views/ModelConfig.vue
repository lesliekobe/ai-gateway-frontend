<script setup>
import { ref, onMounted } from 'vue'
import api from '@/api/index'
import { ElMessage } from 'element-plus'

const configs = ref({ minimax: {}, deepseek: {}, openai: {} })
const loading = ref(false)
const saving = ref(false)

onMounted(async () => {
  loading.value = true
  try {
    const res = await api.get('/admin/keys')
    configs.value = res
  } finally {
    loading.value = false
  }
})

async function handleSave(model) {
  saving.value = true
  try {
    await api.put('/admin/keys', { [model]: configs.value[model] })
    ElMessage.success(`${model.toUpperCase()} 配置已保存`)
  } catch (e) {
    ElMessage.error('保存失败')
  } finally {
    saving.value = false
  }
}

async function handleTest(model) {
  ElMessage.info(`正在测试 ${model.toUpperCase()} 连接...`)
  try {
    await api.post('/ai/test', { model })
    ElMessage.success('连接成功')
  } catch (e) {
    ElMessage.error('连接失败')
  }
}
</script>

<template>
  <div class="model-config">
    <h2>AI 模型配置</h2>
    <el-row :gutter="16">
      <el-col :span="8" v-for="(cfg, model) in configs" :key="model">
        <el-card class="model-card">
          <template #header>
            <div style="display:flex;justify-content:space-between;align-items:center">
              <span style="font-size:18px;font-weight:bold;text-transform:uppercase">{{ model }}</span>
              <el-switch v-model="cfg.enabled" />
            </div>
          </template>
          <el-form label-width="80px" size="small">
            <el-form-item label="API Key">
              <el-input v-model="cfg.api_key" type="password" show-password placeholder="sk-..." />
            </el-form-item>
            <el-form-item label="Base URL">
              <el-input v-model="cfg.base_url" placeholder="https://api.xxx.com" />
            </el-form-item>
            <el-form-item label="代理">
              <el-input v-model="cfg.proxy" placeholder="http://proxy:port (可选)" />
            </el-form-item>
            <el-form-item label="并发上限">
              <el-input-number v-model="cfg.max_concurrent" :min="1" :max="100" />
            </el-form-item>
          </el-form>
          <div style="margin-top:12px;display:flex;gap:8px">
            <el-button type="primary" size="small" :loading="saving" @click="handleSave(model)">保存</el-button>
            <el-button size="small" @click="handleTest(model)">测试连接</el-button>
          </div>
        </el-card>
      </el-col>
    </el-row>
  </div>
</template>

<style scoped>
.model-card { margin-bottom: 16px; }
</style>