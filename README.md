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

* custom controls
    * knobs, both sizes
        * draw arcs in nanovg, should make design easier
    * toggle button
        * match size of small knobs - with or without arcs?
    * light button
        * match size of small knobs - with or without arcs?
    * ports?
        * no, but maybe a custom widget anyway to include output indicator border
* input/output modal with raw values string (comma separated)
    * replace seed
* Github build for all platforms
* Upgrade to C++... 17? 20? What is safe w/VCVrack publishing?
* vcv manual
* better plugin/author/repo name
    * include plugin name at top of svg? what about a logo?
* Update readme with more info
* Publish!
