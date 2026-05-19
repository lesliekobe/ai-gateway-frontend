<script setup>
defineProps({
  title: String,
  value: [Number, String],
  icon: String,
  color: { type: String, default: '#409EFF' }
})
</script>

<template>
  <el-card shadow="hover" class="stats-card">
    <div class="content">
      <el-icon :size="32" :color="color"><component :is="icon" /></el-icon>
      <div class="info">
        <div class="value" :style="{ color }">{{ value }}</div>
        <div class="title">{{ title }}</div>
      </div>
    </div>
  </el-card>
</template>

<style scoped>
.stats-card .content { display: flex; align-items: center; gap: 16px; }
.stats-card .value { font-size: 28px; font-weight: bold; }
.stats-card .title { color: #888; font-size: 14px; margin-top: 4px; }
</style>