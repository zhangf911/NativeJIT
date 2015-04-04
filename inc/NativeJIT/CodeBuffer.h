#pragma once

#include "NativeJIT/JumpTable.h"    // Label parameter and return value.
#include "Temporary/NonCopyable.h"  // Base class.


namespace Allocators
{
    class IAllocator;
}


namespace NativeJIT
{
    class CodeBuffer : public NonCopyable
    {
    public:
        // Allocates a buffer from the allocator.
        // Configures the JumpTable for the specified number of labels and call sites.
        CodeBuffer(Allocators::IAllocator& allocator,
                   unsigned capacity,
                   unsigned maxLabels,
                   unsigned maxCallSites);

        // Frees the buffer.
        virtual ~CodeBuffer();

        // Allocating and resolving jump labels.
        //   Use AllocateLabel() at any time to allocated a label representing a jump target.
        //   Use PlaceLabel() to associate the current buffer position with a label.
        //   Once all code generation is done, invoke Finalize() to patch all of the call 
        //   sites with jump targets. Note that all allocated labels must be placed before 
        //   calling Finalize().
        Label AllocateLabel();
        void PlaceLabel(Label label);

        // TODO: Review whether __forceinline should be added to all Emit*()
        // methods. The following WARNING comment came from CodeBuffer.h within
        // the X64CodeGenerator package. There, CodeBuffer::Emit*() methods were
        // declared and defined in the header file and had __forceinline attribute.
        // WARNING: __forceinline is essential to system performance. Please see
        // note on constant folding in X64CodeGenerator.h.

        // Writes a byte to the current position in the buffer.
        void Emit8(unsigned __int8 x);

        // WARNING: Non portable. Assumes little endian machine architecture.
        void Emit16(unsigned __int16 x);

        // WARNING: Non portable. Assumes little endian machine architecture.
        void Emit32(unsigned __int32 x);

        // WARNING: Non portable. Assumes little endian machine architecture.
        void Emit64(unsigned __int64 x);

        // WARNING: Non portable. Assumes little endian machine architecture.
        template <typename T>
        void EmitValueBytes(T x);

        // Return the size of the buffer, in bytes.
        // TODO: Rename GetCapacity()?
        unsigned BufferSize() const;

        // Returns the address of the start of the buffer.
        // WARNING: Do not use this function to get the address of your generated code.
        // The buffer may contain unwind information before the code starts, and Finalize()
        // must be called before executing the code. 
        // Use X64FunctionGenerator:EntryPoint() instead.
        // TODO: Make protected. Currently available as a public function to enable unit tests.
        unsigned __int8* BufferStart() const;

        // Return the offset of the current write position in the buffer.
        unsigned CurrentPosition() const;

        void Reset(unsigned position);

        // Advances the current write position by byteCount and returns a pointer to the write position
        // before advancing.
        unsigned __int8* Advance(int byteCount);

        template <typename T>
        void AdvanceToAlignment();

        void Fill(unsigned start, unsigned length, unsigned __int8 value);

        // Patches each call site with the correct offset derived from its resolved label.
        void PatchCallSites();

    protected:
        void EmitCallSite(Label label, unsigned size);

    private:
        Allocators::IAllocator& m_allocator;
        unsigned m_capacity;

        unsigned __int8* m_bufferStart;
        unsigned __int8* m_bufferEnd;
        unsigned __int8* m_current;

        JumpTable m_localJumpTable;    // Jumps within a single CodeBuffer.

        // Writes the bits of the argument to the current position in the buffer.
        // WARNING: Non portable. Assumes little endian machine architecture.
        template <typename T>
        __forceinline void EmitBytes(T x);
    };


    //*************************************************************************
    //
    // Template definitions for CodeBuffer.
    //
    //*************************************************************************
    template <typename T>
    void CodeBuffer::AdvanceToAlignment()
    {
        while ( (CurrentPosition() % sizeof(T)) != 0)
        {
            Emit8(0xaa);
        }
    }
}
