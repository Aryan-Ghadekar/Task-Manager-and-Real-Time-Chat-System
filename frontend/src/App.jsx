import { useState, useEffect } from 'react'
import api from './api'
import Login from './components/Login'
import Dashboard from './components/Dashboard'
import './App.css'

function App() {
    const [isAuthenticated, setIsAuthenticated] = useState(false)
    const [user, setUser] = useState(null)
    const [loading, setLoading] = useState(true)

    useEffect(() => {
        // Check if user is already logged in
        const token = localStorage.getItem('token')
        const savedUser = localStorage.getItem('user')

        if (token && savedUser) {
            setIsAuthenticated(true)
            setUser(JSON.parse(savedUser))
        }

        setLoading(false)
    }, [])

    const handleLogin = (userData) => {
        setIsAuthenticated(true)
        setUser(userData)
    }

    const handleLogout = async () => {
        await api.logout()
        setIsAuthenticated(false)
        setUser(null)
    }

    if (loading) {
        return (
            <div className="loading-screen">
                <div className="spinner"></div>
                <p>Loading...</p>
            </div>
        )
    }

    return (
        <div className="app">
            {!isAuthenticated ? (
                <Login onLogin={handleLogin} />
            ) : (
                <Dashboard user={user} onLogout={handleLogout} />
            )}
        </div>
    )
}

export default App
