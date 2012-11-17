#ifndef GAME_MWWORLD_CELLSTORE_H
#define GAME_MWWORLD_CELLSTORE_H

#include <components/esm/records.hpp>

#include <deque>
#include <algorithm>

#include "refdata.hpp"

namespace ESMS
{
    struct ESMStore;
}

namespace MWWorld
{
    class Ptr;

  /// A reference to one object (of any type) in a cell.
  ///
  /// Constructing this with a CellRef instance in the constructor means that
  /// in practice (where D is RefData) the possibly mutable data is copied
  /// across to mData. If later adding data (such as position) to CellRef
  /// this would have to be manually copied across.
  template <typename X>
  struct LiveCellRef
  {
    LiveCellRef(const ESM::CellRef& cref, const X* b = NULL) : base(b), ref(cref),
                                                          mData(ref) {}


    LiveCellRef(const X* b = NULL) : base(b), mData(ref) {}

    // The object that this instance is based on.
    const X* base;

    /* Information about this instance, such as 3D location and
       rotation and individual type-dependent data.
    */
    ESM::CellRef ref;

    /// runtime-data
    RefData mData;
  };

  /// A list of cell references
  template <typename X>
  struct CellRefList
  {
    typedef LiveCellRef<X> LiveRef;
    typedef std::map<int,LiveRef> List;
    List list;

    // Search for the given reference in the given reclist from
    // ESMStore. Insert the reference into the list if a match is
    // found. If not, throw an exception.
    template <typename Y>
    void find(ESM::CellRef &ref, const Y& recList)
    {
      const X* obj = recList.find(ref.mRefID);
      if(obj == NULL)
        throw std::runtime_error("Error resolving cell reference " + ref.mRefID);

      list[ref.mRefnum] = LiveRef(ref, obj);
    }

    LiveRef *find (const std::string& name)
    {
        for (typename std::map<int,LiveRef>::iterator iter (list.begin()); iter!=list.end(); ++iter)
        {
            if (iter->second.mData.getCount() > 0 && iter->second.ref.mRefID == name)
                return &iter->second;
        }

        return 0;
    }

    LiveRef &insert(const LiveRef &item) {
        list[item.ref.mRefnum] = item;
        return list[item.ref.mRefnum];
    }
  };

  /// A list of container references. These references do not track their mRefnumber.
  /// Otherwise, taking 1 of 20 instances of an object would produce multiple objects
  /// with the same reference.
  // TODO: Check how Morrowind does this! Maybe auto-generate references on drop.
  template <typename X>
  struct ContainerRefList
  {
    typedef LiveCellRef<X> LiveRef;
    typedef std::list<LiveRef> List;
    List list;

    // Search for the given reference in the given reclist from
    // ESMStore. Insert the reference into the list if a match is
    // found. If not, throw an exception.
    template <typename Y>
    void find(ESM::CellRef &ref, const Y& recList)
    {
      const X* obj = recList.find(ref.mRefID);
      if(obj == NULL)
        throw std::runtime_error("Error resolving cell reference " + ref.mRefID);

      list.push_back(LiveRef(ref, obj));
    }

    LiveRef *find (const std::string& name)
    {
        for (typename std::list<LiveRef>::iterator iter (list.begin()); iter!=list.end(); ++iter)
        {
            if (iter->mData.getCount() > 0 && iter->ref.mRefID == name)
                return &*iter;
        }

        return 0;
    }

    LiveRef &insert(const LiveRef &item) {
        list.push_back(item);
        return list.back();
    }
  };

  /// A storage struct for one single cell reference.
  class CellStore
  {
  public:

    enum State
    {
        State_Unloaded, State_Preloaded, State_Loaded
    };

    CellStore (const ESM::Cell *cell_);

    const ESM::Cell *cell;
    State mState;
    std::vector<std::string> mIds;

    float mWaterLevel;

    // Lists for each individual object type
    CellRefList<ESM::Activator>         activators;
    CellRefList<ESM::Potion>            potions;
    CellRefList<ESM::Apparatus>         appas;
    CellRefList<ESM::Armor>             armors;
    CellRefList<ESM::Book>              books;
    CellRefList<ESM::Clothing>          clothes;
    CellRefList<ESM::Container>         containers;
    CellRefList<ESM::Creature>          creatures;
    CellRefList<ESM::Door>              doors;
    CellRefList<ESM::Ingredient>        ingreds;
    CellRefList<ESM::CreatureLevList>   creatureLists;
    CellRefList<ESM::ItemLevList>       itemLists;
    CellRefList<ESM::Light>             lights;
    CellRefList<ESM::Tool>              lockpicks;
    CellRefList<ESM::Miscellaneous>     miscItems;
    CellRefList<ESM::NPC>               npcs;
    CellRefList<ESM::Probe>             probes;
    CellRefList<ESM::Repair>            repairs;
    CellRefList<ESM::Static>            statics;
    CellRefList<ESM::Weapon>            weapons;

    void load (const ESMS::ESMStore &store, std::vector<ESM::ESMReader> &esm);

    void preload (const ESMS::ESMStore &store, std::vector<ESM::ESMReader> &esm);

    /// Call functor (ref) for each reference. functor must return a bool. Returning
    /// false will abort the iteration.
    /// \return Iteration completed?
    template<class Functor>
    bool forEach (Functor& functor)
    {
        return
            forEachImp (functor, activators) &&
            forEachImp (functor, potions) &&
            forEachImp (functor, appas) &&
            forEachImp (functor, armors) &&
            forEachImp (functor, books) &&
            forEachImp (functor, clothes) &&
            forEachImp (functor, containers) &&
            forEachImp (functor, creatures) &&
            forEachImp (functor, doors) &&
            forEachImp (functor, ingreds) &&
            forEachImp (functor, creatureLists) &&
            forEachImp (functor, itemLists) &&
            forEachImp (functor, lights) &&
            forEachImp (functor, lockpicks) &&
            forEachImp (functor, miscItems) &&
            forEachImp (functor, npcs) &&
            forEachImp (functor, probes) &&
            forEachImp (functor, repairs) &&
            forEachImp (functor, statics) &&
            forEachImp (functor, weapons);
    }

    bool operator==(const CellStore &cell) {
        return  this->cell->mName == cell.cell->mName &&
                this->cell->mData.mX == cell.cell->mData.mX &&
                this->cell->mData.mY == cell.cell->mData.mY;
    }

    bool operator!=(const CellStore &cell) {
        return !(*this == cell);
    }

    bool isExterior() const {
        return cell->isExterior();
    }

  private:

    template<class Functor, class List>
    bool forEachImp (Functor& functor, List& list)
    {
        for (typename List::List::iterator iter (list.list.begin()); iter!=list.list.end();
            ++iter)
            if (!functor (iter->second.ref, iter->second.mData))
                return false;

        return true;
    }

    /// Run through references and store IDs
    void listRefs(const ESMS::ESMStore &store, std::vector<ESM::ESMReader> &esm);

    void loadRefs(const ESMS::ESMStore &store, std::vector<ESM::ESMReader> &esm);
  };
}

#endif
