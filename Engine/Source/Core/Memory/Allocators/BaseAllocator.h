
namespace Influx
{
	class BaseAllocator
	{
	public:
		virtual ~BaseAllocator() = default;

		virtual void* Allocate(const size_t amount) = 0;
		virtual void Free(void*) = 0;
		virtual void Clear() = 0;

		inline size_t GetTotalSizeInBytes() const { return mTotalSize; }
		inline size_t GetUsedMemory() const { return mMemoryUsed; }
		inline const void* GetBaseAdress() const { return mpBase; }

	protected:
		inline BaseAllocator(const size_t size, const void* pBase) 
			: mpBase{ pBase }, mTotalSize{ size }, mMemoryUsed{}{}

		/* Total size in bytes */
		const size_t mTotalSize;

		/* Pointer to base memory */
		const void* mpBase;
		size_t mMemoryUsed;
	};
}
