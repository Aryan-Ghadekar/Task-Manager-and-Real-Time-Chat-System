import { useState } from 'react'
import api from '../api'
import './Login.css'

function Login({ onLogin }) {
    const [username, setUsername] = useState('')
    const [password, setPassword] = useState('')
    const [error, setError] = useState('')
    const [loading, setLoading] = useState(false)

    const handleSubmit = async (e) => {
        e.preventDefault()
        setError('')
        setLoading(true)

        try {
            const response = await api.login(username, password)

            if (response.success) {
                onLogin(response.user)
            } else {
                setError(response.error || 'Login failed')
            }
        } catch (err) {
            setError('Connection error. Make sure the server is running on port 8081.')
        } finally {
            setLoading(false)
        }
    }

    return (
        <div className="login-container">
            <div className="login-card fade-in">
                <div className="login-header">
                    <h1>Task Manager</h1>
                    <p>Cross-Platform Task Management & Real-Time Chat</p>
                </div>

                <form onSubmit={handleSubmit} className="login-form">
                    <div className="form-group">
                        <label htmlFor="username">Username</label>
                        <input
                            id="username"
                            type="text"
                            value={username}
                            onChange={(e) => setUsername(e.target.value)}
                            placeholder="Enter your username"
                            required
                            autoFocus
                        />
                    </div>

                    <div className="form-group">
                        <label htmlFor="password">Password</label>
                        <input
                            id="password"
                            type="password"
                            value={password}
                            onChange={(e) => setPassword(e.target.value)}
                            placeholder="Enter your password"
                            required
                        />
                    </div>

                    {error && (
                        <div className="error-message">
                            {error}
                        </div>
                    )}

                    <button
                        type="submit"
                        className="btn btn-primary btn-login"
                        disabled={loading}
                    >
                        {loading ? 'Logging in...' : 'Login'}
                    </button>
                </form>

                <div className="login-footer">
                    <p className="demo-accounts">Demo Accounts:</p>
                    <div className="demo-list">
                        <span className="demo-badge">admin / admin</span>
                        <span className="demo-badge">pm1 / pm1</span>
                        <span className="demo-badge">dev1 / dev1</span>
                        <span className="demo-badge">tester1 / tester1</span>
                    </div>
                </div>
            </div>
        </div>
    )
}

export default Login
