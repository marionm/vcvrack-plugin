# sporkbomb VCV Rack plugin

## Entropy Pool

![Entropy Pool](./res/EntropyPool.png)

A sequencer that slices from a pool of random data

### Github integration

The context menu includes an "Integrations..." item, which lets you use a Github token to use a
user's contribution history (the green boxes on their profile page) as the data source.

To load private activity, use a classic token with the `repo` and `read:user` permissions.

The token is *not* saved with patches, but the loaded activity is, so you don't need to re-enter your token every time.

# Development

## Setup

1. Download the Rack SDK to `../Rack-SDK`
  * Or, download anywhere and set the `RACK_DIR` environment variable to its path
2. `make && make install`

## TODO

* better plugin/author name
* Update readme with new screenshot, puddle, more info
    * do this sooner rather than later, but after knob defaults
* attenuverters/cv
* Tooltips
    * clean up formatting w/printf and precision limits - see ScaleParmQuantity
* input/output modal with raw values string (comma separated)
* value tooltips on mouse hover in grid?
* confirm too-short github history works
* Can i support click/drag or right click modal editing *per* cell?
* Another SVG pass
    * subtle background texture - pure flat looks a little odd
    * any border? (like my memoryman, colored front, bare metal texture border)
    * custom knobs?
    * get rid of screws?
    * sporkbomb logo? like, a cartoon bomb cutouts to make tines at the bottom (angled)
* Github build for all platforms
* Upgrade to C++... 17? 20? What is safe w/VCVrack publishing?
* vcv manual? how do those work?
* Publish!
