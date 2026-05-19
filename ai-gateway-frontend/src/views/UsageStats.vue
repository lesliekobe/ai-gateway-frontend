<script setup>
import { ref, onMounted } from 'vue'
import * as echarts from 'echarts'
import api from '@/api/index'

const dateRange = ref([])
const chart1 = ref(null)
const chart2 = ref(null)
const chart3 = ref(null)
const loading = ref(false)

onMounted(async () => {
  loading.value = true
  try {
    const res = await api.get('/admin/usage', { params: { days: 7 } })

    if (chart1.value) {
      echarts.init(chart1.value).setOption({
        title: { text: '每日调用量趋势', left: 'center' },
        tooltip: { trigger: 'axis' },
        xAxis: { type: 'category', data: res.dates || [] },
        yAxis: { type: 'value' },
        series: [{ name: '调用量', type: 'line', data: res.calls || [], smooth: true, areaStyle: {} }]
      })
    }

    if (chart2.value) {
      echarts.init(chart2.value).setOption({
        title: { text: 'Top 10 用户', left: 'center' },
        tooltip: {},
        xAxis: { type: 'category', data: res.top_users?.map(u => u.username) || [] },
        yAxis: { type: 'value' },
        series: [{ name: '调用量', type: 'bar', data: res.top_users?.map(u => u.count) || [] }]
      })
    }

    if (chart3.value) {
      echarts.init(chart3.value).setOption({
        title: { text: '模型分布', left: 'center' },
        tooltip: { trigger: 'item' },
        series: [{ name: '模型', type: 'pie', radius: '60%', data: res.model_dist || [] }]
      })
    }
  } finally {
    loading.value = false
  }
})

async function exportCSV() {
  const res = await api.get('/admin/usage/export', { responseType: 'blob' })
  const url = URL.createObjectURL(new Blob([res]))
  const a = document.createElement('a')
  a.href = url
  a.download = 'usage.csv'
  a.click()
}
</script>

<template>
  <div class="usage-stats">
    <h2>用量统计</h2>
    <el-card>
      <div style="margin-bottom:12px">
        <el-button @click="exportCSV">导出 CSV</el-button>
      </div>
      <el-row :gutter="16">
        <el-col :span="12"><div ref="chart1" style="height:280px;"></div></el-col>
        <el-col :span="12"><div ref="chart2" style="height:280px;"></div></el-col>
      </el-row>
      <el-row :gutter="16" style="margin-top:16px">
        <el-col :span="12"><div ref="chart3" style="height:280px;"></div></el-col>
      </el-row>
    </el-card>
  </div>
</template>