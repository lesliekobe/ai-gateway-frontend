<script setup>
import { ref, computed } from 'vue'
import Sidebar from '@/components/Sidebar.vue'
import Header from '@/components/Header.vue'

const collapsed = ref(false)
</script>

<template>
  <el-container class="layout">
    <el-aside :width="collapsed ? '64px' : '220px'">
      <Sidebar :collapsed="collapsed" />
    </el-aside>
    <el-container>
      <el-header>
        <Header @toggle="collapsed = !collapsed" />
      </el-header>
      <el-main>
        <router-view />
      </el-main>
    </el-container>
  </el-container>
</template>

<style scoped>
.layout { height: 100vh; }
.el-header { background: #fff; border-bottom: 1px solid #e6e6e6; display: flex; align-items: center; padding: 0 16px; }
.el-main { background: #f5f7fa; overflow-y: auto; }
</style>