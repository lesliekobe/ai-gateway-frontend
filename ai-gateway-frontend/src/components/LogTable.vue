<script setup>
defineProps({
  logs: { type: Array, default: () => [] },
  loading: { type: Boolean, default: false },
  pagination: { type: Boolean, default: false },
  total: { type: Number, default: 0 }
})
const emit = defineEmits(['page-change'])

function formatTime(ts) {
  return ts ? new Date(ts * 1000).toLocaleString() : '-'
}
</script>

<template>
  <el-table :data="logs" v-loading="loading" style="width:100%" stripe>
    <el-table-column prop="created_at" label="时间" width="180">
      <template #default="{ row }">{{ formatTime(row.created_at) }}</template>
    </el-table-column>
    <el-table-column prop="username" label="用户" width="120" />
    <el-table-column prop="action" label="操作" width="140">
      <template #default="{ row }">
        <el-tag size="small" type="info">{{ row.action }}</el-tag>
      </template>
    </el-table-column>
    <el-table-column prop="resource" label="资源" width="180" />
    <el-table-column prop="details" label="详情">
      <template #default="{ row }">
        <span style="font-size:12px;color:#666">{{ typeof row.details === 'string' ? row.details : JSON.stringify(row.details) }}</span>
      </template>
    </el-table-column>
    <el-table-column prop="ip" label="IP" width="130" />
    <el-table-column v-if="pagination" label="操作" width="80">
      <template #default>
        <el-button size="small" text>详情</el-button>
      </template>
    </el-table-column>
  </el-table>
</template>