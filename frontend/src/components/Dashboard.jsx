import { useState, useEffect } from 'react'
import api from '../api'
import TaskList from './TaskList'
import Chat from './Chat'
import CreateTask from './CreateTask'
import './Dashboard.css'

function Dashboard({ user, onLogout }) {
    const [tasks, setTasks] = useState([])
    const [activeView, setActiveView] = useState('all') // 'all', 'my', 'overdue'
    const [showCreateTask, setShowCreateTask] = useState(false)
    const [loading, setLoading] = useState(true)

    useEffect(() => {
        loadTasks()
        // Refresh tasks every 5 seconds
        const interval = setInterval(loadTasks, 5000)
        return () => clearInterval(interval)
    }, [activeView])

    const loadTasks = async () => {
        try {
            let response
            if (activeView === 'my') {
                response = await api.getMyTasks()
            } else if (activeView === 'overdue') {
                response = await api.getOverdueTasks()
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
        </div>
    )
}

export default Dashboard
