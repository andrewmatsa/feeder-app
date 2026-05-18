import { Navigate, useLocation } from 'react-router-dom'
import { authStorage } from '../services/api'

export function ProtectedRoute({ children }: { children: React.ReactNode }) {
  const location = useLocation()

  if (!authStorage.getToken()) {
    return <Navigate to="/login" replace state={{ from: location.pathname }} />
  }

  return <>{children}</>
}
