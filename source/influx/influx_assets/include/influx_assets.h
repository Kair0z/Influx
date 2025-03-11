#include "core/file.h"

// todo: get rid of this stl dependency
#include <fstream>

// 
namespace influx::assets
{
	class serializeable
	{
	public:
		void save(const file& file);
		void load(const file& file);

	protected:
		const file& get_file() const;
		const string& get_filename() const;
		bool has_file() const;

	private:
		virtual void on_serialize() const = 0;
		bool m_is_loading;

		std::ofstream m_ofstream;
		std::ifstream m_ifstream;

		file m_file = {};
		string m_name = {};
	};

	// file representing a game project
	class gameproject final : public serializeable
	{
		virtual void on_serialize() const;

	public:
		const string& get_name() const;
	};
}