import './StatCard.css'

function StatCard({ title, value, color = 'blue' }) {
    return (
        <div className={`stat-card stat-${color}`}>
            <div className="stat-header">
                <h3 className="stat-title">{title}</h3>
            </div>
            <div className="stat-body">
                <p className="stat-value">{value}</p>
            </div>
        </div>
    )
}

export default StatCard
