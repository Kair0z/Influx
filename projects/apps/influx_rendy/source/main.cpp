
#include "influx_graphics.h"
#include "rendergraph.h"

extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

using namespace influx;

class clear_pass final : public rendergraph::rgpass
{
public:
	clear_pass()
		: rgpass(rendergraph::e_rgpass_type::graphics, rendergraph::e_rgpass_flags::none)
	{

	}

	virtual void setup(rendergraph::rgbuilder& builder) override
	{
		// ...
	}

	virtual void execute() override
	{
		// ...
	}

private:
};

int main(int argc, char** argv)
{
	using namespace influx;

	graphics::device* device = graphics::device::create(graphics::e_api_type::dx12);
	auto queue = device->create_command_queue();
	auto allocator = device->create_graphics_allocator();
	auto commandlist = device->create_graphics_command_list(allocator);

	rendergraph::rendergraph graph{device};
	graph.add_pass<clear_pass>();
	graph.build();
	graph.execute(commandlist);
}