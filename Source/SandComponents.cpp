#include "SandComponents.hpp"

#include "Snowstorm/Components/ComponentRegistry.hpp"

#include <rttr/registration.h>

namespace Snowstorm
{
	RTTR_REGISTRATION
	{
		using namespace rttr;

		registration::class_<SandWorldComponent>("SandWorldComponent")
		    .constructor()
		    .property("Width", &SandWorldComponent::Width)(metadata("Min", 1))
		    .property("Height", &SandWorldComponent::Height)(metadata("Min", 1));
	}

	AUTO_REGISTER_COMPONENT(SandWorldComponent);
}
