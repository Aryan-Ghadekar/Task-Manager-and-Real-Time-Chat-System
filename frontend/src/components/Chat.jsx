import { useState, useEffect, useRef } from 'react'
import api from '../api'
import './Chat.css'

function Chat({ user }) {
    const [messages, setMessages] = useState([])
    const [newMessage, setNewMessage] = useState('')
    const [loading, setLoading] = useState(true)
    const messagesEndRef = useRef(null)

    useEffect(() => {
        loadMessages()
        // Refresh messages every 3 seconds
        const interval = setInterval(loadMessages, 3000)
        return () => clearInterval(interval)
    }, [])

    useEffect(() => {
        scrollToBottom()
    }, [messages])

    const loadMessages = async () => {
        try {
            const response = await api.getMessages()
            if (response.success && response.data) {
                setMessages(response.data)
            }
        } catch (error) {
            console.error('Failed to load messages:', error)
        } finally {
            setLoading(false)
        }
    }

    const scrollToBottom = () => {
        messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' })
    }

    const handleSendMessage = async (e) => {
        e.preventDefault()
        if (!newMessage.trim()) return

        try {
            await api.sendMessage(newMessage)
            setNewMessage('')
            loadMessages()
        } catch (error) {
            console.error('Failed to send message:', error)
        }
    }

    const formatTime = (timestamp) => {
        const date = new Date(timestamp)
        return date.toLocaleTimeString('en-US', {
            hour: '2-digit',
            minute: '2-digit'
        })
    }

    return (
        <div className="chat-container">
            <div className="panel-header">
                <h2>Team Chat</h2>
            </div>

            <div className="chat-messages">
                {loading ? (
                    <div className="loading-state">
                        <div className="spinner"></div>
                    </div>
                ) : messages.length === 0 ? (
                    <div className="empty-state">
                        <p>No messages yet</p>
                        <span>Start a conversation</span>
                    </div>
                ) : (
                    messages.map((message) => (
                        <div
                            key={message.id}
                            className={`message ${message.senderId === user.id ? 'message-own' : 'message-other'} fade-in`}
                        >
                            <div className="message-header">
                                <span className="message-sender">{message.senderName}</span>
                                <span className="message-time">{formatTime(message.timestamp)}</span>
                            </div>
                            <div className="message-content">{message.content}</div>
                        </div>
                    ))
                )}
                <div ref={messagesEndRef} />
            </div>

            <form onSubmit={handleSendMessage} className="chat-input-form">
                <input
                    type="text"
                    value={newMessage}
                    onChange={(e) => setNewMessage(e.target.value)}
                    placeholder="Type a message..."
                    className="chat-input"
                />
                <button type="submit" className="btn btn-primary send-btn">
                    Send
                </button>
            </form>
        </div>
    )
}

export default Chat
