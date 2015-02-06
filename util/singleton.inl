
template <class T, template <class> class CreationPolicy>
CMutex CSingleton<T, CreationPolicy>::_mutex;

template <class T, template <class> class CreationPolicy>
T* CSingleton<T, CreationPolicy>::_instance = 0;

//----------------------------------------------------------------------

template <class T, template <class> class CreationPolicy>
T* CSingleton<T, CreationPolicy>::Instance (void)
{
    if (0 == _instance)
    {
        //guard
        CScopedLock guard(_mutex);

        if (0 == _instance)
        {
            _instance = CreationPolicy<T>::Create ();
        }
    }

    return _instance;
}
//----------------------------------------------------------------------

template <class T, template <class> class CreationPolicy>
void CSingleton<T, CreationPolicy>::Destroy (void)
{
    return CreationPolicy<T>::Destroy (_instance);
}
//----------------------------------------------------------------------
