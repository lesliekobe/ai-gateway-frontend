<script setup>
import { ref, onMounted } from 'vue'
import api from '@/api/audit'
import LogTable from '@/components/LogTable.vue'

const logs = ref([])
const total = ref(0)
const loading = ref(false)
const filters = ref({ action: '', user_id: null, keyword: '', start_date: '', end_date: '' })

onMounted(async () => { await fetchLogs() })

async function fetchLogs() {
  loading.value = true
  try {
    const params = {}
    if (filters.value.action) params.action = filters.value.action
    if (filters.value.keyword) params.keyword = filters.value.keyword
    if (filters.value.start_date) params.start_date = filters.value.start_date
    if (filters.value.end_date) params.end_date = filters.value.end_date

    const res = await api.getAuditLogs(params)
    logs.value = res.items || res
    total.value = res.total || logs.value.length
  } finally {
    loading.value = false
  }
}

async function exportLogs() {
  const res = await api.exportAuditLogs(filters.value)
  const url = URL.createObjectURL(new Blob([res]))
  const a = document.createElement('a')
  a.href = url
  a.download = 'audit_logs.csv'
  a.click()
}
</script>

<template>
  <div class="audit-log">
    <h2>审计日志</h2>
    <el-card>
      <div class="filters" style="margin-bottom:16px;display:flex;gap:12px;flex-wrap:wrap">
        <el-select v-model="filters.action" placeholder="操作类型" clearable style="width:160px">
          <el-option label="登录" value="LOGIN" />
          <el-option label="登出" value="LOGOUT" />
          <el-option label="AI调用" value="CHAT_REQUEST" />
          <el-option label="设备扫描" value="DEVICE_SCAN" />
          <el-option label="设备绑定" value="DEVICE_BIND" />
          <el-option label="配置变更" value="CONFIG_CHANGE" />
        </el-select>
        <el-input v-model="filters.keyword" placeholder="关键词搜索" clearable style="width:200px" />
        <el-date-picker v-model="filters.start_date" type="date" placeholder="开始日期" style="width:160px" />
        <el-date-picker v-model="filters.end_date" type="date" placeholder="结束日期" style="width:160px" />
        <el-button type="primary" @click="fetchLogs">搜索</el-button>
        <el-button @click="exportLogs">导出 CSV</el-button>
      </div>
      <LogTable :logs="logs" :loading="loading" :pagination="true" :total="total" @page-change="fetchLogs" />
    </el-card>
  </div>
</template>