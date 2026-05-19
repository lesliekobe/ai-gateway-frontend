<script setup>
import { ref, onMounted } from 'vue'
import { useSystemStore } from '@/stores/system'
import StatsCard from '@/components/StatsCard.vue'
import LogTable from '@/components/LogTable.vue'
import * as echarts from 'echarts'

const system = useSystemStore()
const loading = ref(true)

const stats = ref({ total_users: 0, calls_today: 0, online_devices: 0, model_count: 0 })
const recentLogs = ref([])
let chart = null

onMounted(async () => {
  try {
    const data = await system.fetchDashboard()
    stats.value = data.stats || data
    recentLogs.value = data.recent_logs || []

    chart = echarts.init(document.getElementById('usageChart'))
    chart.setOption({
      title: { text: '近7天调用量', left: 'center' },
      tooltip: { trigger: 'axis' },
      xAxis: { type: 'category', data: data.chart?.dates || [] },
      yAxis: { type: 'value' },
      series: [{ name: '调用量', type: 'line', data: data.chart?.calls || [], smooth: true, areaStyle: {} }]
    })
  } finally {
    loading.value = false
  }
})
</script>

<template>
  <div class="dashboard">
    <h2>数据大盘</h2>
    <el-row :gutter="16" class="stats-row">
      <el-col :span="6">
        <StatsCard title="总用户数" :value="stats.total_users" icon="User" color="#409EFF" />
      </el-col>
      <el-col :span="6">
        <StatsCard title="今日调用" :value="stats.calls_today" icon="ChatDotRound" color="#67C23A" />
      </el-col>
      <el-col :span="6">
        <StatsCard title="在线设备" :value="stats.online_devices" icon="Monitor" color="#E6A23C" />
      </el-col>
      <el-col :span="6">
        <StatsCard title="模型配置" :value="stats.model_count" icon="Setting" color="#909399" />
      </el-col>
    </el-row>

    <el-row :gutter="16" style="margin-top: 20px;">
      <el-col :span="16">
        <el-card>
          <div id="usageChart" style="height: 300px;"></div>
        </el-card>
      </el-col>
      <el-col :span="8">
        <el-card title="快捷操作">
          <el-button type="primary" plain style="width:100%;margin-bottom:8px">扫描设备</el-button>
          <el-button type="success" plain style="width:100%;margin-bottom:8px">刷新配置</el-button>
          <el-button type="warning" plain style="width:100%">查看在线用户</el-button>
        </el-card>
      </el-col>
    </el-row>

    <el-card title="最近操作日志" style="margin-top: 20px;">
      <LogTable :logs="recentLogs" :loading="loading" />
    </el-card>
  </div>
</template>