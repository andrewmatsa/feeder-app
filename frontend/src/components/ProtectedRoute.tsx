import { Navigate, useLocation } from 'react-router-dom'
import { authStorage } from '../services/api'
import { useAuthStore } from '../store/authStore'

interface Props {
  children: React.ReactNode
  requireAdmin?: boolean
  guestOnly?: boolean
}

export function ProtectedRoute({ children, requireAdmin = false, guestOnly = false }: Props) {
  const location = useLocation()
  const isAdmin = useAuthStore((s) => s.isAdmin)
  const isAuthenticated = !!authStorage.getToken()

  if (guestOnly) {
    return isAuthenticated ? <Navigate to="/devices" replace /> : <>{children}</>
  }

  if (!isAuthenticated) {
    return <Navigate to="/login" replace state={{ from: location.pathname }} />
  }

  if (requireAdmin && !isAdmin) {
    return <Navigate to="/devices" replace />
  }

  return <>{children}</>
}
