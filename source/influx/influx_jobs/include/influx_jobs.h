#if _DLL
#define INFLUX_JOBS_API __declspec(dllexport)
#else
#define INFLUX_JOBS_API __declspec(dllimport)
#endif

#include "core/basetypes.h"
#include "core/result.h"
#include "core/wait.h"
#include "core/container/vector.h"

namespace influx::jobs
{
	template <typename _t = char>
	using result = influx::result<_t, debug_name>;

	enum class e_logcat { info, warning, error, count };
	typedef void (*log_function)(e_logcat, const char*);

	static constexpr uint64 k_max_jobs = 4096u;
	static constexpr uint64 k_max_dependencies = (k_max_jobs * (k_max_jobs - 1)) / 2;

	using job_id = uint64;
	static constexpr job_id k_invalid_job = (uint64)-1;

	struct job_create_args final
	{

	};

	struct job_data;

	class job_queue final
	{
		vector<job_id>		m_job_id_freelist;
		// vector<job_data>	m_job_data;

	public:
		job_queue();

		INFLUX_JOBS_API
		result<job_id> create_job(const job_create_args& args);

		INFLUX_JOBS_API
		result<> set_dependency(const job_id source, const job_id dest);

		INFLUX_JOBS_API
		result<> queue_job(const job_id job);

	private:
		job_id allocate_id();
		void dealloc_id(const job_id& id);
	};
}