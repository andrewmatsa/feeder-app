import { useEffect } from 'react'
import { BrowserRouter, Navigate, Route, Routes } from 'react-router-dom'
import { Layout } from './components/Layout'
import { ProtectedRoute } from './components/ProtectedRoute'
import { AddDevicePage } from './pages/AddDevicePage'
import { AdminPage } from './pages/AdminPage'
import { DeviceDashboardPage } from './pages/DeviceDashboardPage'
import { DevicesPage } from './pages/DevicesPage'
import { LoginPage } from './pages/LoginPage'
import { RegisterPage } from './pages/RegisterPage'
import { SettingsPage } from './pages/SettingsPage'
import { useAuthStore } from './store/authStore'

function App() {
  const hydrate = useAuthStore((s) => s.hydrate)

  useEffect(() => {
    hydrate()
  }, [hydrate])

  return (
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<Layout />}>
          <Route index element={<Navigate to="/devices" replace />} />
          <Route path="login" element={<ProtectedRoute guestOnly><LoginPage /></ProtectedRoute>} />
          <Route path="register" element={<ProtectedRoute guestOnly><RegisterPage /></ProtectedRoute>} />
          <Route
            path="devices"
            element={
              <ProtectedRoute>
                <DevicesPage />
              </ProtectedRoute>
            }
          />
          <Route
            path="devices/new"
            element={
              <ProtectedRoute>
                <AddDevicePage />
              </ProtectedRoute>
            }
          />
          <Route
            path="devices/:deviceId"
            element={
              <ProtectedRoute>
                <DeviceDashboardPage />
              </ProtectedRoute>
            }
          />
          <Route
            path="settings"
            element={
              <ProtectedRoute>
                <SettingsPage />
              </ProtectedRoute>
            }
          />
          <Route
            path="admin"
            element={
              <ProtectedRoute requireAdmin>
                <AdminPage />
              </ProtectedRoute>
            }
          />
        </Route>
      </Routes>
    </BrowserRouter>
  )
}

export default App
