import { useState, useEffect } from 'react'
import api from '../api'
import TaskList from './TaskList'
import Chat from './Chat'
import CreateTask from './CreateTask'
import StatCard from './StatCard'
import PrivateChat from './PrivateChat'
import './Dashboard.css'

function Dashboard({ user, onLogout }) {
    const [tasks, setTasks] = useState([])
    const [activeView, setActiveView] = useState('all') // 'all', 'my', 'overdue', 'due-soon'
    const [showCreateTask, setShowCreateTask] = useState(false)
    const [showPrivateChat, setShowPrivateChat] = useState(false)
    const [loading, setLoading] = useState(true)
    const [stats, setStats] = useState(null)

    useEffect(() => {
        loadTasks()
        loadStats()
        // Refresh tasks every 5 seconds
        const interval = setInterval(() => {
            loadTasks()
            loadStats()
        }, 5000)
        return () => clearInterval(interval)
    }, [activeView])

    const loadTasks = async () => {
        try {
            let response
            if (activeView === 'my') {
                response = await api.getMyTasks()
            } else if (activeView === 'overdue') {
                response = await api.getOverdueTasks()
            } else if (activeView === 'due-soon') {
                response = await api.getDueSoonTasks(3)
            } else {
                response = await api.getTasks()
            }

            if (response.success && response.data) {
                setTasks(response.data)
            }
        } catch (error) {
            console.error('Failed to load tasks:', error)
        } finally {
            setLoading(false)
        }
    }

    const loadStats = async () => {
        try {
            const response = await api.getStats()
            if (response.success && response.data) {
                setStats(response.data)
            }
        } catch (error) {
            console.error('Failed to load stats:', error)
        }
    }

    const handleTaskCreated = () => {
        setShowCreateTask(false)
        loadTasks()
    }

    const handleTaskUpdated = () => {
        loadTasks()
    }

    return (
        <div className="dashboard">
            {/* Header */}
            <header className="dashboard-header">
                <div className="header-left">
                    <h1 className="app-title">Task Manager</h1>
                    <div className="user-info">
                        <span className="user-name">{user.username}</span>
                        <span className="user-role badge badge-progress">{user.role}</span>
                    </div>
                </div>
                <div className="header-right">
                    <button className="btn btn-secondary" onClick={() => setShowPrivateChat(true)}>
                        💬 Private Messages
                    </button>
                    <button className="btn btn-primary" onClick={() => setShowCreateTask(true)}>
                        + New Task
                    </button>
                    <button className="btn btn-secondary" onClick={onLogout}>
                        Logout
                    </button>
                </div>
            </header>

            {/* Main Content */}
            <div className="dashboard-content">
                {/* Left Panel - Tasks */}
                <div className="tasks-panel">
                    {/* Statistics Panel */}
                    {stats && (
                        <div className="stats-panel">
                            <StatCard
                                title="Total Tasks"
                                value={stats.total}
                                color="blue"
                            />
                            <StatCard
                                title="In Progress"
                                value={stats.inProgress}
                                color="green"
                            />
                            <StatCard
                                title="Overdue"
                                value={stats.overdue}
                                color="red"
                            />
                            <StatCard
                                title="Due Soon"
                                value={stats.dueSoon}
                                color="orange"
                            />
                        </div>
                    )}

                    <div className="panel-header">
                        <h2>Tasks</h2>
                        <div className="view-tabs">
                            <button
                                className={`tab ${activeView === 'all' ? 'active' : ''}`}
                                onClick={() => setActiveView('all')}
                            >
                                All Tasks
                            </button>
                            <button
                                className={`tab ${activeView === 'my' ? 'active' : ''}`}
                                onClick={() => setActiveView('my')}
                            >
                                My Tasks
                            </button>
                            <button
                                className={`tab ${activeView === 'overdue' ? 'active' : ''}`}
                                onClick={() => setActiveView('overdue')}
                            >
                                Overdue
                            </button>
                            <button
                                className={`tab ${activeView === 'due-soon' ? 'active' : ''}`}
                                onClick={() => setActiveView('due-soon')}
                            >
                                Due Soon
                            </button>
                        </div>
                    </div>

                    <div className="panel-content">
                        {loading ? (
                            <div className="loading-state">
                                <div className="spinner"></div>
                                <p>Loading tasks...</p>
                            </div>
                        ) : (
                            <TaskList tasks={tasks} onTaskUpdated={handleTaskUpdated} />
                        )}
                    </div>
                </div>

                {/* Right Panel - Chat */}
                <div className="chat-panel">
                    <Chat user={user} />
                </div>
            </div>

            {/* Create Task Modal */}
            {showCreateTask && (
                <CreateTask
                    onClose={() => setShowCreateTask(false)}
                    onTaskCreated={handleTaskCreated}
                />
            )}

            {/* Private Chat Modal */}
            {showPrivateChat && (
                <PrivateChat
                    currentUser={user}
                    onClose={() => setShowPrivateChat(false)}
                />
            )}
        </div>
    )
}

export default Dashboard
